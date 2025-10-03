/*!
 * \file      bme280.hpp
 * \brief     Responsible for i2c handling
 * \author    Wietse Houwers
 * \date      September 2025
 *
 */

#ifndef I2C_HPP
#define I2C_HPP

#include <cstdint>
#include <string>

//--> i2c class
class I2CDevice {

//--> Public functions
public:
    //--> Constructor
    I2CDevice(int bus, uint8_t address);
    
    //--> Destructor
    ~I2CDevice();

    //--> Read and write functions to and from i2c
    uint8_t  read8(uint8_t reg);
    uint16_t read16(uint8_t reg);
    uint16_t read16_LE(uint8_t reg);
    int16_t  readS16(uint8_t reg);
    int16_t  readS16_LE(uint8_t reg);
    void     write8(uint8_t reg, uint8_t value);

//--> Global variables
private:
    int file;
    uint8_t addr;
};

#endif // I2C_HPP
