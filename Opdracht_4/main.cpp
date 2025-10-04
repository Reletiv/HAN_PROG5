/*!
 * \file      main.cpp
 * \brief     Main entry point for BME280 Test code
 * \author    Wietse Houwers
 * \date      September 2025
 *
 * \details
 * This file contains the setup and main loop for the BME280 Test code.
 * It initializes hardware modules, handles periodic data polling, and writing results over serial
 *
 */

#include "bme280.hpp"
#include <iostream>

//--> Setup
int main() {
    //--> Create sensor object
    BME280 sensor;

    //--> Check if sensor is present
    if (!sensor.begin(0x76, 1)) {
        std::cerr << "BME280 not detected!" << std::endl;
        return 1;
    }

    //--> Loop
    while(1)
	{
		//--> Print current environment information
    		std::cout << "Temperature: " << sensor.readTemperature() << " °C" << std::endl;
    		std::cout << "Pressure: " << sensor.readPressure() << " hPa" << std::endl;
    		std::cout << "Humidity: " << sensor.readHumidity() << " %" << std::endl;

		//--> Sleep
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
    return 0;
}

