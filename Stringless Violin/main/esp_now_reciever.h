#ifndef MAIN_ESP_NOW_RECEIVER_H
#define MAIN_ESP_NOW_RECEIVER_H

// Initialize ESP-NOW receiver (call once from app_main)
void esp_now_receiver_init(void);

// Stop ESP-NOW
void esp_now_receiver_deinit(void);

#endif // MAIN_ESP_NOW_RECEIVER_H