/*!
 * \file      main.cpp
 * \brief     Main entry point for BME280 Test code with MQTT (with authentication)
 * \author    Wietse Houwers
 * \date      September 2025
 *
 * \details
 * This file contains the setup and main loop for the BME280 Test code.
 * It initializes hardware modules, handles periodic data polling, and posts results to MQTT
 *
 */

#include "bme280.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <mqtt/async_client.h>

//--> mqtt setup
const std::string SERVER_ADDRESS{"tcp://192.168.50.95:1883"};
const std::string CLIENT_ID{"WIETSE-PI"};
const std::string TOPIC{"school"};
const int QOS = 1;

//--> mqtt authentication
const std::string MQTT_USERNAME{"school"};
const std::string MQTT_PASSWORD{"Han@2025!"};

//--> Function to publish sensor data
void publishData(mqtt::async_client& client, float temp, float hum, float pres) {
    //--> Build formatted payload
    std::ostringstream payload;
    payload << "{"
            << "\"temperature\":" << temp << ","
            << "\"humidity\":" << hum << ","
            << "\"pressure\":" << pres
            << "}";

    //--> Create mqtt message
    auto msg = mqtt::make_message(TOPIC, payload.str());
    msg->set_qos(QOS);

    //--> Publish and handle errors
    try {
        client.publish(msg)->wait_for(std::chrono::seconds(10));
    } catch (const mqtt::exception& exc) {
        std::cerr << "error publishing" << exc.what() << std::endl;
    }
}

//--> Setup
int main() {
    //--> Create sensor object
    BME280 sensor;

    //--> Check if sensor is present
    if (!sensor.begin(0x76, 1)) {
        std::cerr << "sensor not detected" << std::endl;
        return 1;
    }

    //--> Create mqtt client
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);

    //--> Setup mqtt authentication
    mqtt::connect_options connOpts;
    connOpts.set_clean_session(true);
    connOpts.set_user_name(MQTT_USERNAME);
    connOpts.set_password(MQTT_PASSWORD);

    //--> Connect to mqtt broker
    try {
        client.connect(connOpts)->wait();
        std::cout << "mqtt ok" << std::endl;
    } catch (const mqtt::exception& exc) {
        std::cerr << "mqtt failed" << exc.what() << std::endl;
        return 1;
    }

    //--> Loop
    while(1) {
        //--> Read sensor values
        float temperature = sensor.readTemperature();
        float pressure = sensor.readPressure();
        float humidity = sensor.readHumidity();

        //--> Print current environment information
        std::cout << "Temperature: " << temperature << " °C" << std::endl;
        std::cout << "Pressure: " << pressure << " hPa" << std::endl;
        std::cout << "Humidity: " << humidity << " %" << std::endl;

        //--> Publish sensor data to mqtt
        publishData(client, temperature, humidity, pressure);

        //--> Sleep
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    //--> Disconnect mqtt
    client.disconnect()->wait();
    return 0;
}
