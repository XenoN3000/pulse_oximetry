//
// Created by hosein nahari on 10/15/25.
//

#ifndef ESP32P4_MASTER_IMQTTSUBSCRIBER_HPP
#define ESP32P4_MASTER_IMQTTSUBSCRIBER_HPP
#include <atomic>


#ifdef  __cplusplus
extern "C" {
#endif


#include <esp_event_base.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_event_base.h>
#include <esp_log.h>
#include <esp_event.h>
#include <mqtt_client.h>

#include <driver/gpio.h>
#include <soc/gpio_num.h>
#include <esp_rom_sys.h>
#include <nvs_flash.h>


namespace mqtt_subscriber {

    class IMQTTSubscriber {

    public:
        IMQTTSubscriber(const char *brokerUri, int port, const char *username = nullptr,
                        const char *password = nullptr);
        virtual ~IMQTTSubscriber() = default;

        void start();

        bool isConnected() const;

        void connect();

        void disconnect();
        void publish(const char *topic, const char *data, int len, int qos, int retain);
    protected:
        // Virtual event handlers that can be overridden by derived classes.
        virtual void onConnected(esp_mqtt_event_handle_t event);
        virtual void onDisconnected(esp_mqtt_event_handle_t event);
        virtual void onData(esp_mqtt_event_handle_t event);
        virtual void onError(esp_mqtt_event_handle_t event);
        virtual void onSubscribed(esp_mqtt_event_handle_t event);
        virtual void onUnsubscribed(esp_mqtt_event_handle_t event);
        virtual void onPublished(esp_mqtt_event_handle_t event);
        virtual void onBeforeConnect(esp_mqtt_event_handle_t event);
        virtual void onDefaultEvent(esp_mqtt_event_handle_t event);

        esp_mqtt_client_config_t mqttConfig = {};
        esp_mqtt_client_handle_t client;
        std::atomic<bool> m_isConnected{false};
    private:
        const char *TAG = "MQTT-ABSTRACT";
        static void mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);


        const char *brokerUri;
        int port;
        const char *username;
        const char *password;

    };
}

#ifdef __cplusplus
}
#endif

#endif //ESP32P4_MASTER_IMQTTSUBSCRIBER_HPP
