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

    allData data = {0};
    data.end = 0;

    // Initialize sensors before starting the audio task to guarantee the
    // audio task observes initialized values on its first run.

    pressureSensor(&data);   //data.pressures = {20, 0, 0, 0};

    touchSensor(&data);      //data.positions = {50, 60, 70, 80};

    accelerometer(&data);    //data.bowSpeed = 100;
    mpu6050_init();
    esp_now_receiver_init();

// Create note conversion task pinned to Core 1
    BaseType_t result = xTaskCreatePinnedToCore(
        output,               // Task function
        "Audio output",       // Name
        8192,                 // Stack size in bytes
        &data,                // Parameters
        1,                    // Priority
        NULL,                 // Task handle (optional)
        1                     // Core 1
    );

    if (result != pdPASS) {
        printf("Failed to create task.\n");
    }

    // Debug: confirm the atomic bow speed state after initialization
    int32_t bow_milli_after = atomic_load(&data.bowSpeed_milli);
    printf("main: bowSpeed_milli after init = %ld (%.3f)\n", (long)bow_milli_after, (double)bow_milli_after/1000.0);
    
    
    
    gpio_config_t io_conf = {                   // maps pin bit mask to pin number
        .pin_bit_mask = 1ULL << INPUT_PIN,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "Reading GPIO %d...", INPUT_PIN);
    MPU6050_Data imu;
    
    // Main loop (Core 0)
    int count = 0;
    while (!data.end) {
        update_local_imu();

        int level = gpio_get_level(INPUT_PIN);                     
    // Commented out: verbose GPIO logging
    // ESP_LOGI(TAG, "GPIO%d level: %d", INPUT_PIN, level);       // Read and log the GPIO level
        float dist = calculate_distance(get_local_imu(), get_remote_imu());
        printf("Distance (accel vector): %.2f\n", dist);    
        // printf("Temp: %.2f °C\n", imu.temp);
        touchSensor(&data);

        //printf("[Core %d] Main loop running...\n", xPortGetCoreID());
        //count++;
        //if (count > 5) data.end = 1;  // Stop after 5 loops for testing
        vTaskDelay(pdMS_TO_TICKS(1000)); //2000
    }

    vTaskDelete(NULL);
}