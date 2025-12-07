//
// Created by hosein nahari on 10/15/25.
//

#include "WifiDriver.hpp"

#include <logger.hpp>
#include <MQTTSubscriber.hpp>

#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

namespace wifidriver {

    using wfd = WifiDriver;
    using namespace logger;

    wfd::WifiDriver(const char *ssid, const char *password, uint8_t maxRetry, const char *tag) :
        TAG(tag), ssid(ssid), password(password), maxRetry(maxRetry), retryNum(0), wifiEventGroup(nullptr) {}

    WifiDriver* wfd::New_CreateWiFiDriver(const char *ssid, const char *password, uint8_t maxRetry, const char *tag) {
        return new WifiDriver{
                ssid,
                password,
                maxRetry,
                tag
            };
    }

    WifiDriver wfd::CreateWiFiDriver(const char *ssid, const char *password, uint8_t maxRetry, const char *tag) {
        return {
                ssid,
                password,
                maxRetry,
                tag
            };
    }

    esp_err_t wfd::init() {

        wifiEventGroup = xEventGroupCreate();


        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();


        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));


        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &static_wifi_event_handler,
            this,
            &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &static_wifi_event_handler,
            this,
            &instance_got_ip));

        wifi_config_t wifi_config = {};
        strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), ssid, sizeof(wifi_config.sta.ssid) - 1);
        strncpy(reinterpret_cast<char*>(wifi_config.sta.password), password, sizeof(wifi_config.sta.password) - 1);
        wifi_config.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
        wifi_config.sta.sae_pwe_h2e = ESP_WIFI_SAE_MODE;
        strncpy(reinterpret_cast<char*>(wifi_config.sta.sae_h2e_identifier),
                const_cast<char*>(WIFI_H2E_IDENTIFIER),
                sizeof(wifi_config.sta.sae_h2e_identifier) - 1);
        //
        // strcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), ssid);
        // strcpy(reinterpret_cast<char*>(wifi_config.sta.password), password);


        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(TAG, "wifi_init_sta finished.");

        EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                                               WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);


        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "Connected to AP SSID:%s", ssid);
            return ESP_OK;
        }
        if (bits & WIFI_FAIL_BIT) {
            ESP_LOGI(TAG, "Failed to connect to SSID:%s", ssid);
            return ESP_FAIL;
        }

        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        return ESP_FAIL;

    }

    esp_err_t wfd::init_with_provisioning() {

        wifiEventGroup = xEventGroupCreate();


        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        esp_event_handler_instance_t instance_prov;


        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            WIFI_PROV_EVENT,
            ESP_EVENT_ANY_ID,
            &static_wifi_event_handler,
            this,
            &instance_prov));


        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &static_wifi_event_handler,
            this,
            &instance_any_id
        ));

        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &static_wifi_event_handler,
            this,
            &instance_got_ip
        ));


        wifi_prov_mgr_config_t prov_config = {
                .scheme = wifi_prov_scheme_ble,
                .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
            };

        ESP_ERROR_CHECK(wifi_prov_mgr_init(prov_config));


        bool provisioned = false;
        ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));


        ESP_ERROR_CHECK(wifi_prov_mgr_endpoint_create("mqtt_config"));
        ESP_ERROR_CHECK(
            wifi_prov_mgr_endpoint_register("mqtt_config", mqtt_subscriber::MQTTSubscriber::static_mqtt_config_handler,
                nullptr));


        if (!provisioned) {
            Logger::Instance.info(TAG, STARTING_BLE, PROV_DEVICE_NAME);
            wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

            const char *pop = PROV_PROOF_OF_POSSESSION;
            ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security,pop,PROV_DEVICE_NAME, nullptr));

        }
        else {
            Logger::Instance.info(TAG, ALREADY_PROVISIONED);
            wifi_prov_mgr_deinit(); // Release resources
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            ESP_ERROR_CHECK(esp_wifi_start());
        }

        Logger::Instance.info(TAG, "wifi_init_sta finished.");
        Logger::Instance.info(TAG, "waiting for wifi...");


        EventBits_t bits = xEventGroupWaitBits(wifiEventGroup,
                                               WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                               pdFALSE,
                                               pdFALSE,
                                               portMAX_DELAY);

        return ESP_OK;

    }


    void wfd::wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        }
        else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            if (retryNum < CONFIG_ESP_MAXIMUM_RETRY) {
                esp_wifi_connect();
                retryNum++;
                ESP_LOGI(TAG, "retry to connect to the AP");
            }
            else {
                xEventGroupSetBits(wifiEventGroup, WIFI_FAIL_BIT);
            }
            ESP_LOGI(TAG, "connect to the AP fail");
        }
        else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = (ip_event_got_ip_t*)event_data;
            ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
            retryNum = 0;
            xEventGroupSetBits(wifiEventGroup, WIFI_CONNECTED_BIT);
        }
        else if (event_base == WIFI_PROV_EVENT) {
            switch (event_id) {
            case WIFI_PROV_START:
                Logger::Instance.info(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV:
                {
                    wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t*)event_data;
                    Logger::Instance.info(TAG,
                                          "Received Wi-Fi credentials"
                                          "\n\tSSID     : %s"
                                          "\n\tPassword : %s",
                                          (const char*)wifi_sta_cfg->ssid,
                                          (const char*)wifi_sta_cfg->password);
                    break;
                }
            case WIFI_PROV_CRED_FAIL:
                {
                    wifi_prov_sta_fail_reason_t *reason = (wifi_prov_sta_fail_reason_t*)event_data;
                    Logger::Instance.error(TAG,
                                           "Provisioning failed: %s",
                                           (*reason == WIFI_PROV_STA_AUTH_ERROR)
                                               ? "Wi-Fi Auth Error"
                                               : "Wi-Fi AP Not Found");
                    break;
                }
            case WIFI_PROV_CRED_SUCCESS:
                Logger::Instance.info(TAG, "Provisioning successful");
                break;
            case WIFI_PROV_END:
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
            }
        }

    }


} // wifidriver
