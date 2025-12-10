//
// Created by hosein nahari on 10/15/25.
//

#include "MQTTSubscriber.hpp"

#include <utility>

#include "IMQTTSubscriber.hpp"
#include "../../../../../../../../usr/local/Cellar/esp/esp-idf/components/tcp_transport/private_include/esp_transport_internal.h"

namespace mqtt_subscriber {
    using imqt = IMQTTSubscriber;
    using mqt = MQTTSubscriber;

    using namespace std;

    imqt::IMQTTSubscriber(string brokerUri, int port, string username, string password) :
        client(nullptr),
        brokerUri(std::move(brokerUri)), port(port), username(std::move(username)), password(std::move(password)) {

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

        // load_mqtt_settings();

        if (brokerUri.empty() || port == 0) {
            ESP_LOGI(TAG,"Host And Port Not Set ! \n Host: %s - Port %d", brokerUri.c_str(), port);
            return;
        }
        mqttConfig.broker.address.uri = brokerUri.c_str();
        mqttConfig.broker.address.port = port;
        mqttConfig.network.disable_auto_reconnect = false;
        mqttConfig.credentials.username = username.c_str();
        mqttConfig.credentials.authentication.password = password.c_str();


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

    esp_err_t mqt::mqtt_config_handler(uint32_t session_id, const uint8_t *inbuf, ssize_t inlen, uint8_t **outbuf,
                                       ssize_t *outlen, void *priv_data) {

        if (inbuf) {
            char payload[128];
            if (inlen >= sizeof(payload)) inlen = sizeof(payload) - 1;
            memcpy(payload, inbuf, inlen);
            payload[inlen] = '\0';

            ESP_LOGI(TAG, "Received MQTT Config: %s", payload);

            nvs_handle_t my_handle;
            esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
            if (err == ESP_OK) {
                // Check if there is a delimiter (comma or semicolon)
                char *delim = strchr(payload, ',');
                if (!delim) delim = strchr(payload, ';');

                if (delim) {
                    // Format: HOST,PORT
                    *delim = '\0'; // Split string
                    char *host_str = payload;
                    char *port_str = delim + 1;
                    uint32_t port = atoi(port_str);

                    nvs_set_str(my_handle, "mqtt_host", host_str);
                    nvs_set_u32(my_handle, "mqtt_port", port);

                    nvs_erase_key(my_handle, "mqtt_uri");

                    ESP_LOGI(TAG, "Saved Host: %s, Port: %lu", host_str, port);
                }
                else {
                    nvs_set_str(my_handle, "mqtt_uri", payload);

                    nvs_erase_key(my_handle, "mqtt_host");
                    nvs_erase_key(my_handle, "mqtt_port");

                    ESP_LOGI(TAG, "Saved URI: %s", payload);
                }
                nvs_commit(my_handle);
                nvs_close(my_handle);
            }
        }

        char response[] = "OK";
        *outbuf = (uint8_t*)strdup(response);
        if (*outbuf == nullptr) return ESP_ERR_NO_MEM;
        *outlen = strlen(response) + 1;

        return ESP_OK;
    }

    void imqt::load_mqtt_settings() {
        nvs_handle_t my_handle;
        esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);

        // Reset Globals
        brokerUri = "";
        port = 0;

        if (err == ESP_OK) {
            // Try Loading Host + Port first
            size_t req_size;
            esp_err_t host_err = nvs_get_str(my_handle, "mqtt_host", nullptr, &req_size);
            char *current_mqtt_host = (char*)malloc(req_size > 0 ? req_size : 1); // Allocate at least 1 byte

            char current_mqtt_uri[256]; // A reasonable buffer size for URI

            uint32_t current_mqtt_port;
            if (host_err == ESP_OK && req_size > 0) {
                nvs_get_str(my_handle, "mqtt_host", current_mqtt_host, &req_size);
                nvs_get_u32(my_handle, "mqtt_port", &current_mqtt_port);
               ESP_LOGI(TAG, "Loaded Host: %s, Port: %lu", current_mqtt_host, current_mqtt_port);
                brokerUri = current_mqtt_host;
                port = current_mqtt_port;
            }
            // Fallback to URI
            else if (nvs_get_str(my_handle, "mqtt_uri", nullptr, &req_size) == ESP_OK) {
                if (req_size < sizeof(current_mqtt_uri)) {
                    nvs_get_str(my_handle, "mqtt_uri", current_mqtt_uri, &req_size);
                    ESP_LOGI(TAG, "Loaded URI: %s", current_mqtt_uri);
                    brokerUri = current_mqtt_uri;
                    // Port will be parsed from URI by esp_mqtt_client_init
                }
            }
            else {
               ESP_LOGW(TAG, "No MQTT configuration found in NVS.");
            }
            if (current_mqtt_host) {
                free(current_mqtt_host);
            }
            nvs_close(my_handle);
        }
    }

} // mqtt_subscriber
