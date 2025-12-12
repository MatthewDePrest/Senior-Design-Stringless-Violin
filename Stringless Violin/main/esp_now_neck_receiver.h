#ifndef ESP_NOW_NECK_RECEIVER_H
#define ESP_NOW_NECK_RECEIVER_H

#include <stdint.h>
#include <stdbool.h>

// Packet from neck ESP with pressure and touch sensors
typedef struct {
    int pressures[4];      // Pressure values for 4 strings (0-1023)
    float positions[4];    // Touch position values for 4 strings (0-4095)
    uint32_t timestamp;    // Packet timestamp
} NeckSensorPacket;

void esp_now_neck_receiver_init(void);
NeckSensorPacket* get_neck_sensors(void);
bool is_neck_data_fresh(uint32_t max_age_ms);

#endif
