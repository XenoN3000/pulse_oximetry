//
// Created by hosein nahari on 10/12/25.
//

#ifndef ESP32C6_SLAVE_MESSAGE_H
#define ESP32C6_SLAVE_MESSAGE_H

// #include <ctype.h>

struct ModBusMessage {
    uint16_t adc_value;
    uint16_t cid;
};

#endif //ESP32C6_SLAVE_MESSAGE_H