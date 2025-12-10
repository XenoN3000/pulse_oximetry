//
// Created by hosein nahari on 12/5/25.
//

#include "MAX30102.hpp"

#include <esp_log.h>

namespace max30102 {

    void MAX30102::init(i2c_master_bus_config_t *i2cMasterConfig, i2c_device_config_t *i2cDeviceConfig) {
        ESP_ERROR_CHECK(i2c_new_master_bus(i2cMasterConfig, &busHandle ));
        ESP_ERROR_CHECK(i2c_master_bus_add_device(busHandle, i2cDeviceConfig, &devHandle));

        ESP_LOGI(tag, "%s", RESETTING_SENSOR);
        writeRegister(REG_MODE_CONFIG, 0x40);
        vTaskDelay(pdMS_TO_TICKS(100));

        ESP_LOGI(tag, "%s", CONFIG_SENSOR);

        writeRegister(REG_INTR_ENABLE_1, 0xC0); // Enable interrupts (optional usage)
        writeRegister(REG_INTR_ENABLE_2, 0x00);
        writeRegister(REG_FIFO_WR_PTR, 0x00); // Reset FIFO pointers
        writeRegister(REG_FIFO_OVF_CTR, 0x00);
        writeRegister(REG_FIFO_RD_PTR, 0x00);

        writeRegister(REG_MODE_CONFIG, 0x03); // 0x03 = SpO2 mode (Red + IR)
        writeRegister(REG_SPO2_CONFIG, 0x27); // ADC Range 4096nA, 100Hz sample rate
        writeRegister(REG_LED1_PA, 0x24);     // LED1 (Red) Current ~7mA
        writeRegister(REG_LED2_PA, 0x24);     // LED2 (IR) Current ~7mA
    }

    void MAX30102::writeRegister(uint8_t reg, uint8_t value) {
        uint8_t buffer[2] = {reg, value};
        ESP_ERROR_CHECK(i2c_master_transmit(devHandle, buffer, sizeof(buffer), -1));
    }

    PulseData MAX30102::readValues() {
        uint8_t reg_addr = REG_FIFO_DATA;
        uint8_t data[6]; // 3 bytes Red, 3 bytes IR

        PulseData result = {0, 0};

        esp_err_t ret = i2c_master_transmit_receive(devHandle, &reg_addr, 1, data, 6, -1);
        if (ret != ESP_OK) {
            ESP_LOGE(tag, "I2C transmit failed ( %s )", esp_err_to_name(ret));
            return result;
        }

        // Bytes 0-2: Red Channel
        result.red = ((data[0] & 0x03) << 16) | (data[1] << 8) | data[2];

        // Bytes 3-5: IR Channel
        result.ir = ((data[3] & 0x03) << 16) | (data[4] << 8) | data[5];

        return result;
    }
}
