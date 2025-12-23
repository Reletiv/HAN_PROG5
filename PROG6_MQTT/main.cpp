/*!
 * \file      main.cpp
 * \brief     Main entry point for patient monitor test code with MQTT (with authentication)
 * \author    Wietse Houwers
 * \date      December 2025
 */


/*
* explination of how to use my example mpdule for the prog6 assignment: 
*
* this module acts as an mqtt communicator that updates the current values based on received mqtt commands over the REMOTE_TOPIC.
* The current values are stored in an atomic struct called vitals, (this can also be an external storage by Finn) which allows safe concurrent access from both the main loop and the mqtt callback without race condiotions
* The main loop reads the current values from the vitals struct, displays them, and publishes them to the TOPIC every 5 seconds.
* The mqtt callback listens for messages on the REMOTE_TOPIC and updates the corresponding values in the vitals struct when a message is received.
* the trainee monitor should only have to listen on the TOPIC using an callback. the instructor panel can publish to the REMOTE_TOPIC to change the values.
* the Scenario editor of the other group can also be used to publish to the REMOTE_TOPIC to change the values and look at TOPIC to see the current values
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <atomic>
#include <cctype>
#include <mqtt/async_client.h>

//--> mqtt setup
const std::string SERVER_ADDRESS{"tcp://192.168.50.95:1883"};   // change to "tcp://127.0.0.1:1883" when using local broker 
const std::string CLIENT_ID{"WIETSE-PI"};                       // unique client id
const std::string TOPIC{"current"};                             // topic wich broadcats the current values
const std::string REMOTE_TOPIC{"change/#"};                     // topic used to change the cucrent values
const int QOS = 1;                                              // quality of service             

//--> mqtt authentication
const std::string MQTT_USERNAME{"school"};
const std::string MQTT_PASSWORD{"Han@2025!"};

// --> Use of atomic to allow safe updates from callback
struct VitalSigns {
    std::atomic<float> heartBeat{72.0f};        // beats per minute
    std::atomic<float> bloodPressure{120.0f};   // mmHg
    std::atomic<float> bloodOxygen{98.0f};      // spo2 %
    std::atomic<float> breathSpeed{16.0f};      // breaths per minute
    std::atomic<float> bodyTemperature{36.8f};  // °C
};

// Global vitals instance
VitalSigns vitals;


//--> Function to publish sensor data based on Mqtt example
void publishData(mqtt::async_client& client, float hb, float bp, float oxy, float br, float temp) {
    std::ostringstream payload;
    payload << "{"
            << "\"heartbeat\":" << hb << ","
            << "\"bloodpressure\":" << bp << ","
            << "\"bloodoxygen\":" << oxy << ","
            << "\"breathspeed\":" << br << ","
            << "\"bodytemperature\":" << temp
            << "}";

    auto msg = mqtt::make_message(TOPIC, payload.str());
    msg->set_qos(QOS);

    try {
        client.publish(msg)->wait_for(std::chrono::seconds(10));
    } catch (const mqtt::exception& exc) {
        std::cerr << "error publishing: " << exc.what() << std::endl;
    }
}

// This is the function you asked for: receiveData
void receiveData(const std::string& topic, const std::string& payload) {
    int value;

    try {
        value = std::stoi(payload);
    } catch (...) {
        std::cerr << "Invalid payload: " << payload << std::endl;
        return;
    }

    if (topic == "change/heartbeat") {
        vitals.heartBeat.store(value);
    }
    else if (topic == "change/bloodpressure") {
        vitals.bloodPressure.store(value);
    }
    else if (topic == "change/bloodoxygen") {
        vitals.bloodOxygen.store(value);
    }
    else if (topic == "change/breathspeed") {
        vitals.breathSpeed.store(value);
    }
    else if (topic == "change/bodytemperature") {
        vitals.bodyTemperature.store(value);
    }

    std::cout << "[MQTT] " << topic << " = " << value << std::endl;
}


// MQTT callbacks based on example code
class callback : public virtual mqtt::callback {
public:
    void connected(const std::string& /*cause*/) override {
        std::cout << "mqtt connected (callback)" << std::endl;
    }

    void connection_lost(const std::string& cause) override {
        std::cerr << "mqtt connection lost: " << cause << std::endl;
    }

    void message_arrived(mqtt::const_message_ptr msg) override {
        if (!msg) return;

        // Only handle remote control topic
        receiveData(msg->get_topic(), msg->to_string());
    }

    void delivery_complete(mqtt::delivery_token_ptr /*tok*/) override {}
};

int main() {
    // --> SETUP
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
    mqtt::connect_options connOpts;
    connOpts.set_clean_session(true);
    connOpts.set_user_name(MQTT_USERNAME);
    connOpts.set_password(MQTT_PASSWORD);

    // Set callback
    callback cb;
    client.set_callback(cb);

    try {
        client.connect(connOpts)->wait();
        std::cout << "mqtt ok" << std::endl;

        // Subscribe to remote control topic
        client.subscribe(REMOTE_TOPIC, QOS)->wait();
        std::cout << "subscribed to " << REMOTE_TOPIC << std::endl;

    } catch (const mqtt::exception& exc) {
        std::cerr << "mqtt failed: " << exc.what() << std::endl;
        return 1;
    }

    // --> MAIN LOOP
    while (true) {
        // read sensor data 
        float hb   = vitals.heartBeat;
        float bp   = vitals.bloodPressure;
        float oxy  = vitals.bloodOxygen;
        float br   = vitals.breathSpeed;
        float temp = vitals.bodyTemperature;

        // display data
        std::cout << "Temperature: " << temp << " °C" << std::endl;
        std::cout << "Heart Beat: " << hb << " bpm" << std::endl;
        std::cout << "Blood Pressure: " << bp << " mmHg" << std::endl;
        std::cout << "Blood Oxygen: " << oxy << " %" << std::endl;
        std::cout << "Breath Speed: " << br << " breaths/min" << std::endl;
        
        // publish data
        publishData(client, hb, bp, oxy, br, temp);
        
        // wait before next read
        std::this_thread::sleep_for(std::chrono::seconds(5) );
    }

    // Unreachable, but kept for completeness
    client.disconnect()->wait();
    return 0;
}
