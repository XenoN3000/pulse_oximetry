//
// Created by hosein nahari on 10/15/25.
//

#include "WifiDriver.hpp"

#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

namespace wifidriver {
    using wfd = WifiDriver;

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

        // wifi_config_t wifi_cfg = {};
        // wifi_cfg.sta = {};
        // wifi_cfg.sta.threshold = {};


        // wifi_config_t wifi_config = {
        //         .sta = {
        //
        //             /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
        //              * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
        //              * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
        //              * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
        //              */
        //             .threshold = {
        //                 .authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD
        //             },
        //             .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
        //             // .sae_h2e_identifier = WIFI_H2E_IDENTIFIER,
        //         },
        //     };
        //
        // memset(wifi_config.sta.ssid, '\000', 32);
        // memset(wifi_config.sta.password, '\000', 64);

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
    }


} // wifidriver
