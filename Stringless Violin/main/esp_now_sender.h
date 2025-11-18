#ifndef MAIN_ESP_NOW_SENDER_H
#define MAIN_ESP_NOW_SENDER_H

#include <stdint.h>

// Sensor data packet structure (keep compact for ESP-NOW 250 bytes max)
typedef struct {
    uint8_t pressure[4];      // 4 pressure sensors (0-255 each)
    uint8_t touch_position[4]; // 4 touch positions (0-255 each)
    int16_t bow_speed;        // bow speed in milli-units (-32768 to 32767)
    uint8_t sequence;         // packet sequence number (optional)
} SensorDataPacket;

// Initialize ESP-NOW (call once from app_main)
void esp_now_sender_init(const uint8_t *peer_mac);

// Send sensor data to peer
void esp_now_send_sensor_data(const SensorDataPacket *data);

// Stop ESP-NOW
void esp_now_sender_deinit(void);

#endif // MAIN_ESP_NOW_SENDER_H