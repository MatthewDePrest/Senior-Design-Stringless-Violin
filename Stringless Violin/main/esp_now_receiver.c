#include "esp_now_reciever.h"
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "main.h"
#include "mpu6050.h"
#include <string.h>

static ImuPacket remote_imu;
static ImuPacket local_imu;
static uint32_t last_packet_time = 0;
static uint32_t packet_count = 0;
static uint32_t last_debug_time = 0;

static void esp_now_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len == sizeof(ImuPacket)) {
        memcpy(&remote_imu, data, sizeof(ImuPacket));
    }
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    uint32_t time_since_last = now - last_packet_time;
    last_packet_time = now;
    packet_count++;
    
    // Print connection speed every 1 second
    if (now - last_debug_time > 1000) {
        float packets_per_sec = packet_count / ((float)(now - last_debug_time) / 1000.0f);
        float avg_latency_ms = (float)time_since_last;
        
        ESP_LOGI("ESP_NOW", "RX Speed: %.1f packets/sec | Latency: %.1f ms | Total: %lu packets",
                 packets_per_sec, avg_latency_ms, (unsigned long)packet_count);
        
        last_debug_time = now;
        packet_count = 0;
    }
}

void esp_now_receiver_init(void) {
    nvs_flash_init();
    esp_wifi_init(&(wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT());
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    esp_now_init();
    esp_now_register_recv_cb(esp_now_recv_cb);
}

void update_local_imu(void) {
    mpu6050_read(&local_imu.imu);
}

ImuPacket* get_remote_imu(void) {
    return &remote_imu;
}

ImuPacket* get_local_imu(void) {
    return &local_imu;
}