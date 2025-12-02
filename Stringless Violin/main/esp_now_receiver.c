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

static void esp_now_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len == sizeof(ImuPacket)) {
        memcpy(&remote_imu, data, sizeof(ImuPacket));
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