//
// Created by hosein nahari on 12/5/25.
//

#include "MAX30102.hpp"

namespace max30102 {

    void MAX30102::init(i2c_master_bus_config_t *i2cMasterConfig, i2c_device_config_t *i2cDeviceConfig) {
        ESP_ERROR_CHECK(i2c_new_master_bus(i2cMasterConfig, &busHandle ));
        ESP_ERROR_CHECK(i2c_master_bus_add_device(busHandle, i2cDeviceConfig, &devHandle));

        Logger::Instance.info(tag, RESETTING_SENSOR);
        writeRegister(REG_MODE_CONFIG, 0x40);
        vTaskDelay(pdMS_TO_TICKS(100));

        Logger::Instance.info(tag, CONFIG_SENSOR);

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

    uint32_t MAX30102::readIR() {

        uint8_t reg_addr = REG_FIFO_DATA;
        uint8_t data[6]; // MAX30102 sends 3 bytes Red, 3 bytes IR

        // New API: Transmit (write register addr) + Receive (read data) in one go
        // This handles the "Restart" condition automatically
        esp_err_t ret = i2c_master_transmit_receive(devHandle, &reg_addr, 1, data, 6, -1);

        if (ret != ESP_OK) {
            Logger::Instance.error(tag, I2C_READ_FAILED);
            return 0;
        }

        // Data format:
        // Byte 0-2: Red Channel
        // Byte 3-5: IR Channel
        // We are interested in IR for heartbeat detection
        uint32_t ir = ((data[3] & 0x03) << 16) | (data[4] << 8) | data[5];
        return ir;
    }
}
