//
// Created by hosein nahari on 10/15/25.
//

#ifndef ESP32P4_MASTER_WIFIDRIVER_HPP
#define ESP32P4_MASTER_WIFIDRIVER_HPP

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
// Preprocessor logic from the C example to set up WPA3/SAE modes based on Kconfig


#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define WIFI_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define WIFI_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define WIFI_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

#include <string>

namespace wifidriver {


    constexpr char STARTING_BLE[] = "Starting BLE Provisioning. Name: %s";
    constexpr char PROV_DEVICE_NAME[] = "PROV_MAX30102";
    constexpr char PROV_PROOF_OF_POSSESSION[] = "esp32pop";
    constexpr char ALREADY_PROVISIONED[] = "Already provisioned, starting Wi-Fi...";

    enum WIFI_STATUS {
        WIFI_CONNECTED_BIT = BIT0,
        WIFI_FAIL_BIT      = BIT1
    };


    class WifiDriver {
    public:
        esp_err_t init();

        esp_err_t init_with_provisioning();

        static WifiDriver CreateWiFiDriver(const char *ssid = nullptr, const char *password = nullptr, uint8_t maxRetry = 5,
                                           const char *tag = "WIFI-manager");
        static WifiDriver* New_CreateWiFiDriver(const char *ssid = nullptr, const char *password = nullptr, uint8_t maxRetry = 5,
                                                const char *tag = "WIFI-manager");

        void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
    private:
        const char *TAG;
        const char *ssid;
        const char *password;
        uint8_t maxRetry;
        uint8_t retryNum;
        EventGroupHandle_t wifiEventGroup;


        WifiDriver(const char *ssid, const char *password, uint8_t maxRetry = 5,
                   const char *tag = "WIFI-manager");

        static void static_wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                                              void *event_data) {
            static_cast<WifiDriver*>(arg)->wifiEventHandler(nullptr, event_base, event_id, event_data);
        }


    };

} // wifidriver

#endif //ESP32P4_MASTER_WIFIDRIVER_HPP
