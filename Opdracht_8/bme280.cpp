/*!
 * \file      bme280.cpp
 * \brief     Responsible for environment sensor handling
 * \author    Wietse Houwers
 * \date      December 2025
 *
 * \details
 * Handles i2c communication with the BME280 sensor.
 * Outputs temperature, humidity, and pressure readings.    
 * Based on my own experience from  C code written earlier for NXP developer board.
 * 
 * \note Datasheet: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf
 * All the complex formulas came straight from the Bosch datasheet.
 * 
 */

#include "bme280.hpp"
#include <iostream>
#include <thread>
#include <chrono>


//--> Added to make code more readable for KISS principle
//--> BME280 registers
#define BME280_REG_ID        0xD0
#define BME280_REG_RESET     0xE0
#define BME280_REG_CTRL_HUM  0xF2
#define BME280_REG_STATUS    0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG    0xF5

//--> Device constants
#define BME280_CHIP_ID        0x60
#define BME280_RESET_CMD     0xB6
#define BME280_RESET_DELAY   300

//--> Measurement registers
#define BME280_REG_TEMP_MSB  0xFA
#define BME280_REG_TEMP_LSB  0xFB
#define BME280_REG_TEMP_XLSB 0xFC

#define BME280_REG_PRESS_MSB  0xF7
#define BME280_REG_PRESS_LSB  0xF8
#define BME280_REG_PRESS_XLSB 0xF9

#define BME280_REG_HUM_MSB   0xFD
#define BME280_REG_HUM_LSB   0xFE

//--> Valid ranges
#define BME280_TEMP_MIN     -40.0f
#define BME280_TEMP_MAX      85.0f
#define BME280_PRESS_MIN    300.0f
#define BME280_PRESS_MAX   1100.0f
#define BME280_HUM_MIN        0.0f
#define BME280_HUM_MAX      100.0f






//--> Constructor
BME280::BME280() : dev(nullptr) { }

//--> Initialization
bool BME280::begin(uint8_t addr, int bus) {
    i2caddress = addr;
    try {
        dev = std::make_unique<I2CDevice>(bus, addr);
    } catch (const std::exception &e) {
        //std::cerr << "BME280 init failed: " << e.what() << std::endl; removed for SOLID principle
        return false;
    }

    uint8_t chipid = read8(BME280_REG_ID);
    if (chipid != BME280_CHIP_ID) return false;

    //--> Soft reset
    write8(BME280_REG_RESET, BME280_RESET_CMD);
    std::this_thread::sleep_for(std::chrono::milliseconds(BME280_RESET_DELAY));

    //--> Read factory calibration data
    readCalibration();

    //--> Humidity oversampling x1
    write8(BME280_REG_CTRL_HUM, 0x01);

    //--> Normal mode, oversampling x1 (temp+press)
    write8(BME280_REG_CTRL_MEAS, 0x27);

    //--> Config register (standby, filter off)
    write8(BME280_REG_CONFIG, 0x00);

    return true;
}

//--> Low-level I2C functions
uint8_t BME280::read8(uint8_t reg) { return dev->read8(reg); }
uint16_t BME280::read16(uint8_t reg) { return dev->read16(reg); }
uint16_t BME280::read16_LE(uint8_t reg) { return dev->read16_LE(reg); }
int16_t BME280::readS16(uint8_t reg) { return dev->readS16(reg); }
int16_t BME280::readS16_LE(uint8_t reg) { return dev->readS16_LE(reg); }
void BME280::write8(uint8_t reg, uint8_t val) { dev->write8(reg, val); }

// Read calibration data
void BME280::readCalibration() {
    dig_T1 = read16_LE(0x88);
    dig_T2 = readS16_LE(0x8A);
    dig_T3 = readS16_LE(0x8C);
    dig_P1 = read16_LE(0x8E);
    dig_P2 = readS16_LE(0x90);
    dig_P3 = readS16_LE(0x92);
    dig_P4 = readS16_LE(0x94);
    dig_P5 = readS16_LE(0x96);
    dig_P6 = readS16_LE(0x98);
    dig_P7 = readS16_LE(0x9A);
    dig_P8 = readS16_LE(0x9C);
    dig_P9 = readS16_LE(0x9E);

    dig_H1 = read8(0xA1);
    dig_H2 = readS16_LE(0xE1);
    dig_H3 = read8(0xE3);
    dig_H4 = (read8(0xE4) << 4) | (read8(0xE5) & 0x0F);
    dig_H5 = (read8(0xE6) << 4) | (read8(0xE5) >> 4);
    dig_H6 = static_cast<int8_t>(read8(0xE7));
}

//--> Read temperature in Celsius
float BME280::readTemperature() {
    //--> Read raw 20-bit ADC temperature data (stored in 3 registers)
    int32_t adc_T = (read8(BME280_REG_TEMP_MSB) << 12) | (read8(BME280_REG_TEMP_LSB) << 4) | (read8(BME280_REG_TEMP_XLSB) >> 4);

    //--> First temperature compensation step
    float var1 = ((adc_T / 16384.0f) - (dig_T1 / 1024.0f)) * dig_T2;

    //--> Second temperature compensation step
    float var2 = (((adc_T / 131072.0f) - (dig_T1 / 8192.0f)) *
                  ((adc_T / 131072.0f) - (dig_T1 / 8192.0f))) * dig_T3;

    //--> Final temperature in Celsius
    float temp = (var1 + var2) / 5120.0f;

    //--> Return last valid if out of range for DRY principle
    temp = validOrLast(temp, BME280_TEMP_MIN, BME280_TEMP_MAX, lastTemperature);
    lastTemperature = temp;
    return temp;
}

//--> Read temperature and update t_fine (used for pressure/humidity)
int32_t BME280::updateTFine() {
    //--> Read raw 20-bit ADC temperature data (stored in 3 registers)
    int32_t adc_T = (read8(BME280_REG_TEMP_MSB) << 12) | (read8(BME280_REG_TEMP_LSB) << 4) | (read8(BME280_REG_TEMP_XLSB) >> 4);

    //--> First temperature compensation step
    float var1 = ((adc_T / 16384.0f) - (dig_T1 / 1024.0f)) * dig_T2;

    //--> Second temperature compensation step
    float var2 = (((adc_T / 131072.0f) - (dig_T1 / 8192.0f)) *
                  ((adc_T / 131072.0f) - (dig_T1 / 8192.0f))) * dig_T3;

    //--> Fine temperature (used for pressure/humidity too)
    int32_t t_fine = static_cast<int32_t>(var1 + var2);
    return t_fine;
}

//--> Read pressure in hpa
float BME280::readPressure() {
    //--> Must read temperature first to update t_fine from datasheet
    int32_t t_fine = updateTFine();

    //--> Raw 20-bit ADC pressure data
    int32_t adc_P = (read8(BME280_REG_PRESS_MSB) << 12) | (read8(BME280_REG_PRESS_LSB) << 4) | (read8(BME280_REG_PRESS_XLSB) >> 4);

    //--> Long black magic math from datasheet...
    int64_t var1, var2, p;

    var1 = static_cast<int64_t>(t_fine) - 128000;
    var2 = var1 * var1 * dig_P6;
    var2 = var2 + ((var1 * dig_P5) << 17);
    var2 = var2 + (static_cast<int64_t>(dig_P4) << 35);
    var1 = ((var1 * var1 * dig_P3) >> 8) + ((var1 * dig_P2) << 12);
    var1 = ((((int64_t)1 << 47) + var1) * dig_P1) >> 33;
    if (var1 == 0) return lastPressure;

    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (dig_P9 * (p >> 13) * (p >> 13)) >> 25;
    var2 = (dig_P8 * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (dig_P7 << 4);

    //--> Convert pressure to hpa
    float pressure = static_cast<float>(p) / 25600.0f;

    //--> Return last valid if out of range for DRY principle
    pressure = validOrLast(pressure, BME280_PRESS_MIN, BME280_PRESS_MAX, lastPressure);
    lastPressure = pressure;
    return pressure;
}

//--> Read humidity in %
float BME280::readHumidity() {
    //--> Must read temperature first to update t_fine
    int32_t t_fine = updateTFine();

    //--> Raw 16-bit ADC humidity data
    int32_t adc_H = (read8(BME280_REG_HUM_MSB) << 8) | read8(BME280_REG_HUM_LSB);

    //--> Long black magic math from datasheet...
    int32_t v_x1_u32r = t_fine - 76800;
    v_x1_u32r = (((((adc_H << 14) - (dig_H4 << 20) - (dig_H5 * v_x1_u32r)) + 16384) >> 15) *
                 (((((((v_x1_u32r * dig_H6) >> 10) *
                      (((v_x1_u32r * dig_H3) >> 11) + 32768)) >> 10) + 2097152) *
                   dig_H2 + 8192) >> 14));
    v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * dig_H1) >> 4);

    //--> make sure of valid range
    if (v_x1_u32r < 0) v_x1_u32r = 0;
    if (v_x1_u32r > 419430400) v_x1_u32r = 419430400;

    //--> Convert humidity to percentage
    float humidity = (v_x1_u32r >> 12) / 1024.0f;

    //--> Return last valid if out of range for DRY principle
    humidity = validOrLast(humidity, BME280_HUM_MIN, BME280_HUM_MAX, lastHumidity);
    lastHumidity = humidity;
    return humidity;
}

//--> Helper function to return last valid reading if current is out of range
float BME280::validOrLast(float value, float min, float max, float last) {
    if (std::isnan(value) || value < min || value > max) {
        return last;
    }
    return value;
}
