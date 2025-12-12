#include "esp_now_neck_receiver.h"
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "NECK_ESP";

// Neck ESP - pressure and touch sensor data
static NeckSensorPacket remote_neck_sensors;
static uint32_t last_neck_packet_time = 0;
static uint32_t neck_packet_count = 0;
static uint32_t last_neck_debug_time = 0;

static void esp_now_neck_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    // Handle neck ESP packets (pressure + touch sensor data)
    if (len == sizeof(NeckSensorPacket)) {
        memcpy(&remote_neck_sensors, data, sizeof(NeckSensorPacket));
        uint32_t time_since_last = now - last_neck_packet_time;
        last_neck_packet_time = now;
        neck_packet_count++;
        
        // Print neck sensor stats every 1 second
        if (now - last_neck_debug_time > 1000) {
            float packets_per_sec = neck_packet_count / ((float)(now - last_neck_debug_time) / 1000.0f);
            
            ESP_LOGI(TAG, "RX Speed: %.1f pkt/sec | Latency: %.1f ms | press:[%d,%d,%d,%d] | pos:[%.0f,%.0f,%.0f,%.0f]",
                     packets_per_sec, (float)time_since_last,
                     remote_neck_sensors.pressures[0], remote_neck_sensors.pressures[1], 
                     remote_neck_sensors.pressures[2], remote_neck_sensors.pressures[3],
                     remote_neck_sensors.positions[0], remote_neck_sensors.positions[1],
                     remote_neck_sensors.positions[2], remote_neck_sensors.positions[3]);
            
            last_neck_debug_time = now;
            neck_packet_count = 0;
        }
    } else {
        ESP_LOGW(TAG, "Received packet with unexpected size: %d (expected %d)", len, sizeof(NeckSensorPacket));
    }
}

void esp_now_neck_receiver_init(void) {
    // Note: WiFi should already be initialized by main ESP-NOW receiver
    // This just registers an additional callback
    
    esp_err_t ret = esp_now_register_recv_cb(esp_now_neck_recv_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register neck receiver callback: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Neck sensor receiver initialized");
    }
}

NeckSensorPacket* get_neck_sensors(void) {
    return &remote_neck_sensors;
}

bool is_neck_data_fresh(uint32_t max_age_ms) {
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t age = now - last_neck_packet_time;
    return (age < max_age_ms);
}
