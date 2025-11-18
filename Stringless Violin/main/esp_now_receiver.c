#include "esp_now_reciever.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "esp_now_receiver";

static bool g_esp_now_ready = false;

// Callback when data is received from XIAO
static void esp_now_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    // Your XIAO struct from the sketch
    typedef struct {
        float ax;
        float ay;
        float az;
        float gx;
        float gy;
        float gz;
        float temp;
    } struct_message;

    if (len != sizeof(struct_message)) {
        ESP_LOGW(TAG, "Received %d bytes, expected %zu", len, sizeof(struct_message));
        return;
    }

    struct_message incomingData;
    memcpy(&incomingData, data, sizeof(struct_message));

    // Print unpacked sensor data
    ESP_LOGI(TAG, "=== IMU Data from XIAO ===");
    ESP_LOGI(TAG, "Accel X: %.2f, Y: %.2f, Z: %.2f", 
             incomingData.ax, incomingData.ay, incomingData.az);
    ESP_LOGI(TAG, "Gyro X: %.2f, Y: %.2f, Z: %.2f", 
             incomingData.gx, incomingData.gy, incomingData.gz);
    ESP_LOGI(TAG, "Temp: %.2f C", incomingData.temp);
}

void esp_now_receiver_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(esp_now_recv_cb));

    g_esp_now_ready = true;
    ESP_LOGI(TAG, "ESP-NOW receiver initialized, waiting for XIAO IMU data...");
}

void esp_now_receiver_deinit(void) {
    if (g_esp_now_ready) {
        esp_now_deinit();
        esp_wifi_stop();
        g_esp_now_ready = false;
        ESP_LOGI(TAG, "ESP-NOW receiver deinitialized");
    }
}