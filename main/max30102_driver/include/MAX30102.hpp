//
// Created by hosein nahari on 12/5/25.
//

#ifndef PULS_OXIMETRY_MAX30102_HPP
#define PULS_OXIMETRY_MAX30102_HPP


#include <logger.hpp>
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


namespace max30102 {

    using namespace logger;


    constexpr i2c_master_bus_config_t i2cMasterDefaultConfig() {
        i2c_master_bus_config_t i2c_mst_config = {};
        i2c_mst_config.clk_source = I2C_CLK_SRC_DEFAULT;
        i2c_mst_config.i2c_port = I2C_NUM_0;
        i2c_mst_config.scl_io_num = GPIO_NUM_22;
        i2c_mst_config.sda_io_num = GPIO_NUM_21;
        i2c_mst_config.glitch_ignore_cnt = 7;
        i2c_mst_config.flags.enable_internal_pullup = true;

        return i2c_mst_config;
    }

    constexpr i2c_device_config_t i2cDeviceDefaultConfig() {
        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = 0x57;
        dev_cfg.scl_speed_hz = 100000;

        return dev_cfg;
    }


    constexpr char RESETTING_SENSOR[] = "Resetting Sensor...";
    constexpr char CONFIG_SENSOR[] = "Configuring Sensor...";
    constexpr char I2C_READ_FAILED[] = "I2C Read Failed";


#define I2C_MASTER_SCL_IO   22
#define I2C_MASTER_SDA_IO   21
#define I2C_PORT_NUM        I2C_NUM_0
#define MAX30102_ADDR       0x57

    enum REG_MODE {
        REG_INTR_ENABLE_1 = 0x02,
        REG_INTR_ENABLE_2,
        REG_FIFO_WR_PTR,
        REG_FIFO_OVF_CTR,
        REG_FIFO_RD_PTR,
        REG_FIFO_DATA,
        REG_MODE_CONFIG = 0x09,
        REG_SPO2_CONFIG,
        REG_LED1_PA = 0x0C,
        REG_LED2_PA,
    };

    struct PulseData {
        uint32_t red;
        uint32_t ir;
    };


    class MAX30102 {

    private:
        const char *tag;
        i2c_master_bus_handle_t busHandle;
        i2c_master_dev_handle_t devHandle;

        void writeRegister(uint8_t reg, uint8_t value);
    public:
        MAX30102(i2c_master_bus_config_t *i2cMasterConfig, i2c_device_config_t *i2cDeviceConfig,
                 const char *tag = "MAX30102") : tag(tag) {

            init(i2cMasterConfig, i2cDeviceConfig);
        }

        void init(i2c_master_bus_config_t *i2cMasterConfig, i2c_device_config_t *i2cDeviceConfig);

        PulseData readValues();
    };

} // max30102

#endif //PULS_OXIMETRY_MAX30102_HPP
