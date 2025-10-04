/*!
 * \file      bme280.h
 * \brief     Responsible for environment sensor handling
 * \author    Wietse Houwers
 * \date      September 2025
 *
 */
#ifndef BME280_H
#define BME280_H
#include <Arduino.h>
#include <Wire.h>

//--> BME280 sensor class
class BME280 {
  //-> Public functions
  public:
    //--> Constructor
    BME280();      

    //--> Initialization with default address                    
    bool begin(uint8_t addr = 0x76);   

    //--> Read sensor values
    float readTemperature();       
    float readHumidity();
    float readPressure();

  //-> Private functions and variables
  private:
    //--> i2c address 
    uint8_t i2caddress;
    
    //--> Calibration data (black magic straight from bosch datasheet, stored in sensor at the factory)
    uint16_t dig_T1;
    int16_t  dig_T2, dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint8_t  dig_H1, dig_H3;
    int16_t  dig_H2, dig_H4, dig_H5, dig_H6;

    int32_t t_fine; //--> variable from bosch datasheet

    //--> Store last known valid readings
    float lastTemperature = 20.0; 
    float lastHumidity = 50.0;
    float lastPressure = 1000;


    //--> I2C helper functions for Wirelibrarey
    uint8_t read8(uint8_t reg);
    uint16_t read16(uint8_t reg);
    int16_t readS16(uint8_t reg);
    uint16_t read16_LE(uint8_t reg);
    int16_t readS16_LE(uint8_t reg);
    void readCalibration();
    void write8(uint8_t reg, uint8_t value);
};

#endif // BME280_H
