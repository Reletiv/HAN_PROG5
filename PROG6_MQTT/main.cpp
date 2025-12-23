/*!
 * \file      main.cpp
 * \brief     Main entry point for BME280 Test code with MQTT (with authentication)
 * \author    Wietse Houwers
 * \date      September 2025
 */

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <atomic>
#include <cctype>
#include <mqtt/async_client.h>

//--> mqtt setup
const std::string SERVER_ADDRESS{"tcp://192.168.50.95:1883"};
const std::string CLIENT_ID{"WIETSE-PI"};
const std::string TOPIC{"school"};          // publish topic
const std::string REMOTE_TOPIC{"remote"};   // control topic (remote PC publishes here)
const int QOS = 1;

//--> mqtt authentication
const std::string MQTT_USERNAME{"school"};
const std::string MQTT_PASSWORD{"Han@2025!"};

// --> Use of atomic to allow safe updates from callback
std::atomic<int> delaySeconds{5};

//--> Function to publish sensor data
void publishData(mqtt::async_client& client, float temp, float hum, float pres) {
    std::ostringstream payload;
    payload << "{"
            << "\"temperature\":" << temp << ","
            << "\"humidity\":" << hum << ","
            << "\"pressure\":" << pres
            << "}";

    auto msg = mqtt::make_message(TOPIC, payload.str());
    msg->set_qos(QOS);

    try {
        client.publish(msg)->wait_for(std::chrono::seconds(10));
    } catch (const mqtt::exception& exc) {
        std::cerr << "error publishing: " << exc.what() << std::endl;
    }
}

// Try to extract an integer delay from either "10" or {"delay":10}
int parseDelaySeconds(const std::string& payload) {
    // 1) If payload is plain number: "10"
    try {
        size_t idx = 0;
        int v = std::stoi(payload, &idx);
        // allow trailing whitespace
        while (idx < payload.size() && std::isspace(static_cast<unsigned char>(payload[idx]))) idx++;
        if (idx == payload.size()) return v;
    } catch (...) {
        // fall through
    }

    // 2) Very small JSON-ish parse: find "delay" then parse number after ':'
    auto keyPos = payload.find("delay");
    if (keyPos == std::string::npos) return -1;

    auto colonPos = payload.find(':', keyPos);
    if (colonPos == std::string::npos) return -1;

    // skip spaces
    size_t i = colonPos + 1;
    while (i < payload.size() && std::isspace(static_cast<unsigned char>(payload[i]))) i++;

    // optional quote
    if (i < payload.size() && payload[i] == '"') i++;

    // parse integer
    size_t start = i;
    if (i < payload.size() && (payload[i] == '+' || payload[i] == '-')) i++;
    while (i < payload.size() && std::isdigit(static_cast<unsigned char>(payload[i]))) i++;

    if (i == start || (i == start + 1 && (payload[start] == '+' || payload[start] == '-')))
        return -1;

    try {
        return std::stoi(payload.substr(start, i - start));
    } catch (...) {
        return -1;
    }
}

// This is the function you asked for: receiveData
void receiveData(const std::string& payload) {
    int newDelay = parseDelaySeconds(payload);

    // Validate range (choose what you want)
    if (newDelay < 1 || newDelay > 3600) {
        std::cerr << "[remote] invalid delay payload: '" << payload
                  << "' (allowed 1..3600 seconds)" << std::endl;
        return;
    }

    delaySeconds.store(newDelay);
    std::cout << "[remote] delay updated to " << newDelay << " seconds" << std::endl;
}

// MQTT callbacks
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
        if (msg->get_topic() == REMOTE_TOPIC) {
            receiveData(msg->to_string());
        }
    }

    void delivery_complete(mqtt::delivery_token_ptr /*tok*/) override {}
};

int main() {
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

    while (true) {
        float temperature = 10.10f; 
        float pressure    = 1013.25f;
        float humidity    = 50.0f;

        std::cout << "Temperature: " << temperature << " °C" << std::endl;
        std::cout << "Pressure: "    << pressure    << " hPa" << std::endl;
        std::cout << "Humidity: "    << humidity    << " %" << std::endl;

        publishData(client, temperature, humidity, pressure);

        // Use the latest delay value
        int s = delaySeconds.load();

        // Optional: sleep in small chunks so remote changes take effect faster than waiting full 's'
        for (int i = 0; i < s * 10; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            // if delay changed, restart timing immediately
            if (delaySeconds.load() != s) break;
        }
    }

    // Unreachable, but kept for completeness
    client.disconnect()->wait();
    return 0;
}
