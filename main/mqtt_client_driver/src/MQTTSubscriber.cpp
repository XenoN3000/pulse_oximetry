//
// Created by hosein nahari on 10/15/25.
//

#include "MQTTSubscriber.hpp"
#include "IMQTTSubscriber.hpp"
#include "../../../../../../../../usr/local/Cellar/esp/esp-idf/components/tcp_transport/private_include/esp_transport_internal.h"

namespace mqtt_subscriber {
    using imqt = IMQTTSubscriber;


    imqt::IMQTTSubscriber(const char *brokerUri, int port, const char *username, const char *password) :
        client(nullptr),
        brokerUri(brokerUri), port(port), username(username), password(password) {

        this->start();
    }

    void imqt::publish(const char *topic, const char *data, int len, int qos, int retain) {
        if (client == nullptr) {
            ESP_LOGE(TAG, "client is null");
            return;
        }
        esp_mqtt_client_publish(client, topic, data, len, qos, retain);
        ESP_LOGI(TAG, "publish success %s, %s", data, topic);
    }


    void imqt::connect() {

        if (client != nullptr) {
            ESP_LOGI(TAG, "Attempting to manually reconnect MQTT client...");
            esp_err_t err = esp_mqtt_client_reconnect(client);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Manual reconnect failed: %s", esp_err_to_name(err));
            }
            else {
                // m_isConnected = true;
            }
        }
        else {
            ESP_LOGE(TAG, "Cannot reconnect, client is not initialized.");
        }
    }


    void imqt::mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
        auto *mqttClient = static_cast<IMQTTSubscriber*>(handler_args);
        auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);

        switch (static_cast<esp_mqtt_event_id_t>(event_id)) {
        case MQTT_EVENT_CONNECTED:
            {
                mqttClient->m_isConnected = true;
                mqttClient->onConnected(event);
                break;
            }

        case MQTT_EVENT_DISCONNECTED:
            {
                mqttClient->onDisconnected(event);
                mqttClient->m_isConnected = false;
                break;
            }
        case MQTT_EVENT_DATA: mqttClient->onData(event);
            break;
        case MQTT_EVENT_ERROR: mqttClient->onError(event);
            break;
        case MQTT_EVENT_SUBSCRIBED: mqttClient->onSubscribed(event);
            break;
        case MQTT_EVENT_UNSUBSCRIBED: mqttClient->onUnsubscribed(event);
            break;
        case MQTT_EVENT_PUBLISHED: mqttClient->onPublished(event);
            break;
        case MQTT_EVENT_BEFORE_CONNECT: mqttClient->onBeforeConnect(event);
            break;
        default: mqttClient->onDefaultEvent(event);
            break;
        }
    }

    void imqt::disconnect() {
        esp_err_t err = esp_mqtt_client_disconnect(this->client);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Manual disconnect failed: %s", esp_err_to_name(err));
        }


    }

    void imqt::start() {


        mqttConfig.broker.address.uri = brokerUri;
        mqttConfig.broker.address.port = port;
        mqttConfig.network.disable_auto_reconnect = false;
        mqttConfig.credentials.username = username;
        mqttConfig.credentials.authentication.password = password;


        ESP_LOGI(TAG, "--- DUMPING MQTT CONFIG ---");
        ESP_LOGI(TAG, "Hostname: %s", mqttConfig.broker.address.hostname ? mqttConfig.broker.address.hostname : "NULL");
        ESP_LOGI(TAG, "Port: %d", (int)mqttConfig.broker.address.port);
        ESP_LOGI(TAG, "Transport: %d (TCP=0, SSL=1, WS=2, WSS=3)", (int)mqttConfig.broker.address.transport);
        ESP_LOGI(TAG, "URI: %s", mqttConfig.broker.address.uri ? mqttConfig.broker.address.uri : "NULL");
        ESP_LOGI(TAG, "Username: %s", mqttConfig.credentials.username ? mqttConfig.credentials.username : "NULL");
        ESP_LOGI(TAG, "---------------------------");

        ESP_LOGI(TAG, "init client");
        this->client = esp_mqtt_client_init(&mqttConfig);
        ESP_LOGI(TAG, "mqtt client is null chet %d", client == nullptr);
        esp_mqtt_client_register_event(client,
                                       static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID),
                                       mqttEventHandler,
                                       this);
        esp_mqtt_client_start(client);
    }


    bool imqt::isConnected() const {
        return m_isConnected;
    }


    void imqt::onConnected(esp_mqtt_event_handle_t event) {
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    }

    void imqt::onDisconnected(esp_mqtt_event_handle_t event) {
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    }

    void imqt::onData(esp_mqtt_event_handle_t event) { ESP_LOGI(TAG, "MQTT_EVENT_DATA"); }

    void imqt::onError(esp_mqtt_event_handle_t event) { ESP_LOGE(TAG, "MQTT_EVENT_ERROR"); }

    void imqt::onSubscribed(esp_mqtt_event_handle_t event) {
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    }

    void imqt::onUnsubscribed(esp_mqtt_event_handle_t event) {
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    }

    void imqt::onPublished(esp_mqtt_event_handle_t event) {
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    }

    void imqt::onBeforeConnect(esp_mqtt_event_handle_t event) { ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT"); }

    void imqt::onDefaultEvent(esp_mqtt_event_handle_t event) { ESP_LOGI(TAG, "Other event id:%d", event->event_id); }


} // mqtt_subscriber
