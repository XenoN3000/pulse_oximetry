//
// Created by hosein nahari on 10/15/25.
//

#ifndef ESP32P4_MASTER_MQTTSUBSCRIBER_HPP
#define ESP32P4_MASTER_MQTTSUBSCRIBER_HPP

#include "IMQTTSubscriber.hpp"

namespace mqtt_subscriber {

    class MQTTSubscriber : public IMQTTSubscriber {
    public:
        static MQTTSubscriber* New_MQTTSubscriber(const char *brokerUri = CONFIG_MQTT_BROKER_URI, int port = CONFIG_MQTT_BROKER_PORT, const char *username = CONFIG_MQTT_ACCESS_TOKEN,
                                                  const char *password = "") {
            return new MQTTSubscriber(brokerUri, port, username, password);
        }

        static MQTTSubscriber CreateMQTTSubscriber(const char *brokerUri = CONFIG_MQTT_BROKER_URI, int port = CONFIG_MQTT_BROKER_PORT, const char *username = CONFIG_MQTT_ACCESS_TOKEN,
                                                  const char *password = "") {
            return {brokerUri, port, username, password};
        }
    protected:
        void onConnected(esp_mqtt_event_handle_t event) override {
            // Subscribe to the control topic
            esp_mqtt_client_subscribe(event->client, "v1/devices/me/telemetry", 0);

            // Publish a "hello world" message to a different topic
            // publish("v1/devices/me/telemetry", "{adc_value:1}", 0, 0, 0);
        }
    private:
        MQTTSubscriber(const char *brokerUri, int port, const char *username, const char *password = "") :
            IMQTTSubscriber(brokerUri, port, username, password), TAG("MQTT-CLASS") {
        }

        [[maybe_unused]] const char *TAG;

    };

} // mqtt_subscriber

#endif //ESP32P4_MASTER_MQTTSUBSCRIBER_HPP
