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
 * \note Serial baud rate is 115200, i2c address is 0x76 
 */

#include <Arduino.h>
#include "bme280.h"

BME280 sensor;

//--> Setup
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; ///--> wait for serial port to connect. 
  }

  if (!sensor.begin(0x76)) {
    Serial.println("BME280 not found!");
    while (1);
  }
  Serial.println("BME280 initialized");
}

//--> Main loop
void loop() {
  //--> Read and print sensor values
  Serial.println("----------------------------------");
  Serial.print("Temperature: "); Serial.println(sensor.readTemperature());
  Serial.print("Humidity: "); Serial.println(sensor.readHumidity());
  Serial.print("Pressure: "); Serial.println(sensor.readPressure());
  Serial.println("----------------------------------");
  Serial.println("");
  //--> Wait 5 seconds before next reading
  delay(5000);
}