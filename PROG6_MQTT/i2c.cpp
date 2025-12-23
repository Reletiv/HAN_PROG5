/*!
 * \file      i2c.cpp
 * \brief     Responsible for i2c handling
 * \author    Wietse Houwers
 * \date      September 2025
 *
 */

#include "i2c.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <iostream>

//--> Constructor
I2CDevice::I2CDevice(int bus, uint8_t address) : addr(address) {
    std::string filename = "/dev/i2c-" + std::to_string(bus);
    file = open(filename.c_str(), O_RDWR);
    if (file < 0) throw std::runtime_error("Cannot open I2C bus: " + filename);

    if (ioctl(file, I2C_SLAVE, addr) < 0) throw std::runtime_error("Cannot select I2C device at address 0x" + std::to_string(addr));
}

//--> Destructor
I2CDevice::~I2CDevice() {
    if (file >= 0) close(file);
}

//--> Read 1 byte from i2c register
uint8_t I2CDevice::read8(uint8_t reg) {
    if (write(file, &reg, 1) != 1) throw std::runtime_error("I2C write failed (read8)");
    uint8_t val;
    if (read(file, &val, 1) != 1) throw std::runtime_error("I2C read failed (read8)");
    return val;
}

//--> read 2 byte from i2c register
uint16_t I2CDevice::read16(uint8_t reg) {
    if (write(file, &reg, 1) != 1) throw std::runtime_error("I2C write failed (read16)");
    uint8_t buf[2];
    if (read(file, buf, 2) != 2) throw std::runtime_error("I2C read failed (read16)");
    return (buf[0] << 8) | buf[1];
}

//--> read 2 byte from i2c register and flip to little endian 
uint16_t I2CDevice::read16_LE(uint8_t reg) {
    uint16_t val = read16(reg);
    return (val >> 8) | (val << 8);
}

//--> read 2 byte from i2c register and see them as signed 
int16_t I2CDevice::readS16(uint8_t reg) {
    return static_cast<int16_t>(read16(reg));
}

//--> read 2 byte from i2c register and do both of above
int16_t I2CDevice::readS16_LE(uint8_t reg) {
    return static_cast<int16_t>(read16_LE(reg));
}

//--> write 1 byte to i2c register
void I2CDevice::write8(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = { reg, value };
    if (write(file, buf, 2) != 2) throw std::runtime_error("I2C write failed (write8)");
}
