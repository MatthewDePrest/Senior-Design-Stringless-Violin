#include "esp_now_neck_receiver.h"
#include "main.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include <string.h>
#include <stdatomic.h>

static const char *TAG = "ESPNOW_RX";

typedef struct {
    uint8_t pin8;
    uint8_t pin18;
    uint8_t pin4;
    uint8_t pin5;
    uint16_t analog6;
} PinPacket;

// Pointer to shared struct (provided by main)
static allData *g_data = NULL;

void esp_now_set_data_ptr(allData *ptr)
{
    g_data = ptr;
}

static void recv_cb(const esp_now_recv_info_t *info,
                    const uint8_t *data,
                    int len)
{
    if (len != sizeof(PinPacket)) {
        ESP_LOGW(TAG, "Unexpected packet size: %d", len);
        return;
    }

    if (!g_data) {
        ESP_LOGW(TAG, "g_data not set yet");
        return;
    }

    PinPacket pkt;
    memcpy(&pkt, data, sizeof(pkt));

    ESP_LOGI(TAG,
        "RX from %02X:%02X:%02X:%02X:%02X:%02X | "
        "D8=%d D18=%d D4=%d D5=%d A6=%d",
        info->src_addr[0], info->src_addr[1], info->src_addr[2],
        info->src_addr[3], info->src_addr[4], info->src_addr[5],
        pkt.pin8, pkt.pin18, pkt.pin4, pkt.pin5, pkt.analog6
    );

    // ---------------------------------------------
    //  WRITE INTO YOUR PROJECT GLOBAL STRUCT
    // ---------------------------------------------
    g_data->pressures[3] = pkt.pin8;
    g_data->pressures[2] = pkt.pin18;
    g_data->pressures[1] = pkt.pin4;
    g_data->pressures[0] = pkt.pin5;

    // analog → bow speed (milli-units)
    atomic_store(&g_data->positions[0], (float)pkt.analog6);
    atomic_store(&g_data->positions[1], (float)pkt.analog6);
    atomic_store(&g_data->positions[2], (float)pkt.analog6);
    atomic_store(&g_data->positions[3], (float)pkt.analog6);

    // Signal new data
    g_data->end = 1;
}

void esp_now_neck_receiver_init(void)
{
    ESP_LOGI(TAG, "Initializing ESP-NOW receiver...");

    // Must init Wi-Fi in station mode
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Init ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());

    // Register callback
    ESP_ERROR_CHECK(esp_now_register_recv_cb(recv_cb));

    ESP_LOGI(TAG, "ESP-NOW receiver ready");
}
