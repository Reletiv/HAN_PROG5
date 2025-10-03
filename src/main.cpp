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
 * \note Serial settings are 115200, 8N1
 */

#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

// Setup: Default platformio code template
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. 
  }
}

// Main loop: Default platformio code template
void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here: Default platformio code template
int myFunction(int x, int y) {
  return x + y;
}