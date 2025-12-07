//
// Created by hosein nahari on 10/15/25.
//

#ifndef ESP32P4_MASTER_MQTTSUBSCRIBER_HPP
#define ESP32P4_MASTER_MQTTSUBSCRIBER_HPP

#include <utility>

#include "IMQTTSubscriber.hpp"

namespace mqtt_subscriber {

    class MQTTSubscriber : public IMQTTSubscriber {
    public:
        static MQTTSubscriber* New_MQTTSubscriber(string brokerUri = CONFIG_MQTT_BROKER_URI,
                                                  int port = CONFIG_MQTT_BROKER_PORT,
                                                  string username = CONFIG_MQTT_ACCESS_TOKEN,
                                                 string password = "") {
            return new MQTTSubscriber(std::move(brokerUri), port, std::move(username), std::move(password));
        }

        static MQTTSubscriber CreateMQTTSubscriber(string brokerUri = CONFIG_MQTT_BROKER_URI,
                                                   int port = CONFIG_MQTT_BROKER_PORT,
                                                   string username = CONFIG_MQTT_ACCESS_TOKEN,
                                                   string password = "") {
            return {std::move(brokerUri), port, std::move(username), std::move(password)};
        }


        static esp_err_t static_mqtt_config_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen, uint8_t **outbuf,
                                             ssize_t *outlen, void *priv_data) {
            return
                static_cast<MQTTSubscriber*>(priv_data)->mqtt_config_handler(session_id,
                                                                             inbuf,
                                                                             inlen,
                                                                             outbuf,
                                                                             outlen,
                                                                             priv_data);
        }
    protected:
        void onConnected(esp_mqtt_event_handle_t event) override {
            // Subscribe to the control topic
            esp_mqtt_client_subscribe(event->client, "telemetry/sensor/raw", 0);

            // Publish a "hello world" message to a different topic
            // publish("v1/devices/me/telemetry", "{adc_value:1}", 0, 0, 0);
        }
    private:
        MQTTSubscriber(string brokerUri, int port, string username, string password = "") :
            IMQTTSubscriber(std::move(brokerUri), port, std::move(username), std::move(password)), TAG("MQTT-CLASS") {}

        esp_err_t mqtt_config_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen, uint8_t **outbuf,
                                      ssize_t *outlen, void *priv_data);


        [[maybe_unused]] const char *TAG;

    };

} // mqtt_subscriber

#endif //ESP32P4_MASTER_MQTTSUBSCRIBER_HPP
