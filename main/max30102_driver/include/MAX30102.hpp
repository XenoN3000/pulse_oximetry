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


    constexpr char RESETTING_SENSOR[] = "Resetting Sensor...";
    constexpr char CONFIG_SENSOR[] = "Configuring Sensor...";
    constexpr char I2C_READ_FAILED[] = "I2C Read Failed";


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

    class MAX30102 {

    private:
        const char *tag;
        i2c_master_bus_handle_t busHandle;
        i2c_master_dev_handle_t devHandle;

        void writeRegister(uint8_t reg, uint8_t value) ;
    public:
        MAX30102(i2c_master_bus_config_t *i2cMasterConfig, i2c_device_config_t *i2cDeviceConfig,
                 const char *tag = "MAX30102") : tag(tag) {

            init(i2cMasterConfig, i2cDeviceConfig);
        }

        void init(i2c_master_bus_config_t *i2cMasterConfig, i2c_device_config_t *i2cDeviceConfig);

        uint32_t readIR();
    };

} // max30102

#endif //PULS_OXIMETRY_MAX30102_HPP
