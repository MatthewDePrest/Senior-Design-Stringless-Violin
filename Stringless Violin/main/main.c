#include <stdio.h>
// Include the generated SDK config so CONFIG_* macros (like CONFIG_FREERTOS_HZ)
// are defined for FreeRTOS macros such as pdMS_TO_TICKS.
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
#include "main.h"


void app_main(void)
{
    printf("[Core %d] Main app started!\n", xPortGetCoreID());

    allData data = {0};
    data.end = 0;

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

    pressureSensor(&data);   //data.pressures = {20, 0, 0, 0};
    touchSensor(&data);      //data.positions = {50, 60, 70, 80};
    accelerometer(&data);    //data.bowSpeed = 100;

    // Print all values saved in allData
    printf("Pressures: [%d, %d, %d, %d]\n", data.pressures[0], data.pressures[1], data.pressures[2], data.pressures[3]);
    printf("Positions: [%.1f, %.1f, %.1f, %.1f]\n", data.positions[0], data.positions[1], data.positions[2], data.positions[3]);
    printf("Bow Speed: %.1f\n", data.bowSpeed);
    printf("\nOutputs: [%.1f Hz, %.1f Hz, %.1f Hz, %.1f Hz]\n\n", data.stringsFreqs[0], data.stringsFreqs[1], data.stringsFreqs[2], data.stringsFreqs[3]);

    // Main loop (Core 0)
    int count = 0;
    while (!data.end) {
        printf("[Core %d] Main loop running...\n", xPortGetCoreID());
        count++;
        if (count > 5) data.end = 1;  // Stop after 5 loops for testing
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    vTaskDelete(NULL);
} 