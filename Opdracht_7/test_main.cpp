/*!
 * \file      main.cpp
 * \brief     Basic Unit Test for BME280 sensor
 * \author    Wietse Houwers
 * \date      October 2025
 *
 * \details
 * This file performs a simple functional test of the BME280 sensor.
 * It checks initialisation of the sensor and the values:  temperature
 * and humidity readings change when the sensor is touched.
 */

#include "bme280.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

int main() {
    // Initialise sensor
    BME280 sensor;
    std::cout << "Initializing BME280" << std::endl;
    if (!sensor.begin(0x76, 1)) {
        std::cerr << "\nTEST FAILED: BME280 not detected! Check sensor wiring or setup.\n" << std::endl;
        return 1;
    }
    std::cout << "Sensor Initialised.\n";
    std::cout << "Hold your finger on the sensor when the program asks, It will check responsiveness of the sensor!\n";

    // Take a baseline reading
    double baseTemp = sensor.readTemperature();
    double baseHum = sensor.readHumidity();

    // Print Current values
    std::cout << "Current temperature: " << baseTemp << " °C\n";
    std::cout << "Current humidity: " << baseHum << " %\n";
    std::cout << "\nTouch the sensor for 10 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // Take new reading
    double newTemp = sensor.readTemperature();
    double newHum = sensor.readHumidity();

    // Print New values
    std::cout << "New temperature: " << newTemp << " °C\n";
    std::cout << "New humidity: " << newHum << " %\n";

    // Check for reasonable change
    bool tempChanged = std::fabs(newTemp - baseTemp) > 5;
    bool humChanged  = std::fabs(newHum - baseHum) > 5;

    // Decide if the test fails or passes
    if (tempChanged || humChanged) {
        std::cout << "\nTEST PASSED: Sensor is responsive\n";
    } else {
        std::cout << "\nTEST FAILED: Readings did not change. Check sensor wiring or setup.\n";
    }

    return 0;
}
