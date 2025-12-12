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
static NeckSensorPacket remote_neck_sensors;
static allData *g_data = NULL;
static uint32_t last_packet_time = 0;
static uint32_t last_neck_packet_time = 0;
static uint32_t last_neck_log_time = 0;
static uint32_t packet_count = 0;
static bool neck_connected = false;

typedef struct {
    uint8_t pin8;
    uint8_t pin18;
    uint8_t pin4;
    uint8_t pin5;
    uint16_t analog6;
} PinPacket;

void esp_now_set_data_ptr(allData *ptr) {
    g_data = ptr;
}

static void esp_now_recv_cb(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    // Log ALL received packets for debugging (every 2 sec)
    if (info && info->src_addr) {
        static uint32_t last_any_log = 0;
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (now - last_any_log > 2000) {
            ESP_LOGI("ESP_NOW", "RX from %02X:%02X:%02X:%02X:%02X:%02X | len=%d (ImuPacket=%d, NeckPacket=%d)",
                     info->src_addr[0], info->src_addr[1], info->src_addr[2],
                     info->src_addr[3], info->src_addr[4], info->src_addr[5],
                     len, (int)sizeof(ImuPacket), (int)sizeof(NeckSensorPacket));
            last_any_log = now;
        }
    }

    // Discriminate packet types by size
    if (len == (int)sizeof(ImuPacket)) {
        memcpy(&remote_imu, data, sizeof(ImuPacket));
        if (g_data) {
            // optional: could map IMU into g_data if needed
        }
    } else if (len == (int)sizeof(PinPacket)) {
        if (!g_data) {
            ESP_LOGW("ESP_NOW", "PinPacket received before data ptr set");
            return;
        }
        PinPacket pkt;
        memcpy(&pkt, data, sizeof(PinPacket));

        // Digital pins come in as 0/1; scale to 0/1023 like the old touchSensor_task did
        g_data->pressures[3] = pkt.pin8  ? 1023 : 0;
        g_data->pressures[2] = pkt.pin18 ? 1023 : 0;
        g_data->pressures[1] = pkt.pin4  ? 1023 : 0;
        g_data->pressures[0] = pkt.pin5  ? 1023 : 0;

        // Map analog reading to positions (mirroring old touchSensor_task behavior)
        g_data->positions[0] = (float)pkt.analog6;
        g_data->positions[1] = (float)pkt.analog6;
        g_data->positions[2] = (float)pkt.analog6;
        g_data->positions[3] = (float)pkt.analog6;

        // Do NOT touch bowSpeed_milli here; it stays with its existing producer

        // Mirror the old touchSensor_task debug printout for live visibility
        printf("\rraw=%4u  E=%d  A=%d  D=%d  G=%d    \x1b[0K",
               (unsigned)pkt.analog6,
               g_data->pressures[3], g_data->pressures[2],
               g_data->pressures[1], g_data->pressures[0]);
        fflush(stdout);
    } else if (len == (int)sizeof(NeckSensorPacket)) {
        memcpy(&remote_neck_sensors, data, sizeof(NeckSensorPacket));
        last_neck_packet_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (!neck_connected) {
            neck_connected = true;
            ESP_LOGI("ESP_NOW", "Neck ESP connected: receiving sensor packets");
        }
        // Log neck data every ~1s
        if (last_neck_packet_time - last_neck_log_time > 1000) {
            last_neck_log_time = last_neck_packet_time;
            ESP_LOGI("ESP_NOW", "Neck pkt press=[%d,%d,%d,%d] pos=[%.0f,%.0f,%.0f,%.0f]",
                     remote_neck_sensors.pressures[0], remote_neck_sensors.pressures[1],
                     remote_neck_sensors.pressures[2], remote_neck_sensors.pressures[3],
                     remote_neck_sensors.positions[0], remote_neck_sensors.positions[1],
                     remote_neck_sensors.positions[2], remote_neck_sensors.positions[3]);
        }
    } else {
        return;
    }

    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    (void)now;
    packet_count++;
    last_packet_time = now;
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

// Provide accessors for neck sensor data from unified RX path
NeckSensorPacket* get_neck_sensors(void) {
    return &remote_neck_sensors;
}