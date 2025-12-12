#include "esp_now_neck_receiver.h"
#include "esp_now_neck_receiver.h"
#include "esp_log.h"

static const char *TAG = "NECK_ESP";

// RX is handled centrally in esp_now_receiver.c to avoid double registration.
// Keep this file as an interface and simple init stub.

void esp_now_neck_receiver_init(void) {
    ESP_LOGI(TAG, "Neck sensor receiver active (central RX callback)");
}
static NeckSensorPacket remote_neck_sensors;
