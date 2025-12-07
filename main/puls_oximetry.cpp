#include "puls_oximetry.hpp"
#include "MAX30102.hpp"

using namespace std;

extern "C" [[noreturn]] void app_main(void) {

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    i2c_master_bus_config_t i2cMasterConfig = max30102::i2cMasterDefaultConfig();
    i2c_device_config_t i2cDeviceConfig = max30102::i2cDeviceDefaultConfig();

    auto wifi = wifidriver::WifiDriver::New_CreateWiFiDriver();
    auto sensor = max30102::MAX30102(&i2cMasterConfig, &i2cDeviceConfig);
    auto mqtt = mqtt_subscriber::MQTTSubscriber::New_MQTTSubscriber();

    wifi->init_with_provisioning();
    mqtt->start();


    std::string payload = "";
    // max30102::PulseData pulseData{};
    while (true) {
        auto sensorData = sensor.readValues();

        mqtt->connect();

        if (sensorData.ir > 10000) {

            // sprintf(payload, "red : %lu, ir : %lu", sensorData.red, sensorData.ir);
            payload = "red : " + to_string(sensorData.red);
            payload += " ir : " + to_string(sensorData.ir);


            mqtt->publish("telemetry/sensor/raw", payload.c_str(), static_cast<int>(payload.length()), 0, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(20));

    }
}
