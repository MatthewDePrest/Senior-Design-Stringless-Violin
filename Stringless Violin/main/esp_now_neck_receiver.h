#pragma once
#include <stdint.h>
#include "esp_now.h"

#ifdef __cplusplus
extern "C" {
#endif

// allData is defined in main.h; forward declare to avoid duplicate definitions
typedef struct allData allData;

// Packet sent from the neck ESP containing finger pressures and positions
typedef struct {
    int pressures[4];
    float positions[4];
} NeckSensorPacket;

// Initializes the ESP-NOW receiver dedicated to neck sensor packets
void esp_now_neck_receiver_init(void);
void esp_now_set_data_ptr(allData *ptr);

#ifdef __cplusplus
}
#endif
