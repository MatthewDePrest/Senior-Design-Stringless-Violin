#include "esp_mac.h"
#include "driver/gpio.h"
#include <stdio.h>
#include <inttypes.h>
#include "mpu6050.h"
#include "distance_calc.h"
#include "esp_now_reciever.h"   // <-- Add this line

#include "sdkconfig.h"
// Provide fallbacks for static analysis/builds where sdkconfig.h isn't
// available or the CONFIG_FREERTOS_HZ macro isn't defined. These
// defaults are safe for analysis and won't affect normal ESP-IDF builds
// where sdkconfig.h provides correct values.
#ifndef CONFIG_FREERTOS_HZ
#define CONFIG_FREERTOS_HZ 1000
#endif

#ifndef portTICK_PERIOD_MS
#define portTICK_PERIOD_MS (1000 / CONFIG_FREERTOS_HZ)
#endif
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#if !defined(pdMS_TO_TICKS)
// Fallback for static analyzers or build setups where the SDK config
// macro (CONFIG_FREERTOS_HZ) isn't visible. The real pdMS_TO_TICKS is
// provided by FreeRTOS headers when the build environment is configured.
#define pdMS_TO_TICKS(ms) ((TickType_t) ((ms) / portTICK_PERIOD_MS))
#endif
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "main.h"

#define INPUT_PIN   GPIO_NUM_4
#define OUTPUT_PIN  GPIO_NUM_5

static const char *TAG = "GPIO";
static uint32_t s_last_remote_update_tick = 0;
static bool s_remote_connected = false;


/*
// Minimal test (commented out): prints a heartbeat every second.
// Uncomment for isolated serial/MCU checks.
//
void app_main(void)
{
    printf("[Core %d] Minimal test started!\n", xPortGetCoreID());
    int ticks = 0;
    while (1) {
        printf("tick %d\n", ticks++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
*/

void app_main(void)
{
    printf("[Core %d] Main app started!\n", xPortGetCoreID());
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    printf("[S3 MAC Address] %02X:%02X:%02X:%02X:%02X:%02X\n", 
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    allData data = {0};
    data.end = 0;   

    pressureSensor(&data);
    accelerometer(&data);
    // mpu6050_init();
    esp_now_receiver_init();
    xTaskCreate(touchSensor_task, "touchSensor_task", 4096, &data, 10, NULL);
    
    // Create output task (HIGHEST priority - audio critical)
    xTaskCreate(output, "output", 8192, &data, 25, NULL);
    
    BaseType_t result = xTaskCreatePinnedToCore(
        output,
        "Audio output",
        8192,
        &data,
        1,
        NULL,
        1
    );

    if (result != pdPASS) {
        printf("Failed to create task.\n");
    }

    int32_t bow_milli_after = atomic_load(&data.bowSpeed_milli);
    printf("main: bowSpeed_milli after init = %ld (%.3f)\n", (long)bow_milli_after, (double)bow_milli_after/1000.0);

    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << INPUT_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "Reading GPIO %d...", INPUT_PIN);

    // Main loop (Core 0)
    uint32_t loop_count = 0;
    while (!data.end) {
        update_local_imu();
        int level = gpio_get_level(INPUT_PIN);

        ImuPacket *local = get_local_imu();
        ImuPacket *remote = get_remote_imu();
        
        // Check if remote data has been updated
        uint32_t now = xTaskGetTickCount();
        static uint32_t last_check = 0;
        
        // Detect new data from ESP-XIAO by checking if remote gyro changed
        if (remote->imu.gx != 0.0f || remote->imu.gy != 0.0f || remote->imu.gz != 0.0f) {
            if (now != s_last_remote_update_tick) {
                s_last_remote_update_tick = now;
                s_remote_connected = true;
            }
        }

        float dist = calculate_distance(local, remote);

        // Print connectivity status every 5 seconds (5000 ms / 1000 ms = 5 loops)
        if (loop_count % 5 == 0) {
            if (s_remote_connected) {
                printf("[CONNECTED] Distance: %.2f | Local Gyro: [%.2f, %.2f, %.2f] | Remote Gyro: [%.2f, %.2f, %.2f]\n",
                       dist,
                       local->imu.gx, local->imu.gy, local->imu.gz,
                       remote->imu.gx, remote->imu.gy, remote->imu.gz);
            } else {
                printf("[DISCONNECTED] Waiting for ESP32-XIAO...\n");
            }
        }

        // touchSensor(&data);
        loop_count++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}