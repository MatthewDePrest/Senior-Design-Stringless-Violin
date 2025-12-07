#include "main.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "touchSensor";

void adc_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
}

void touchSensor_task(void *pvParameters) {
    allData *data = (allData *)pvParameters;
    
    // Initialize ADC once at startup
    adc_init();
    ESP_LOGI(TAG, "Touch sensor task started");
    
    vTaskDelay(pdMS_TO_TICKS(100));  // Let ADC settle
    
    int read_count = 0;
    
    while (!data->end) {
        // Read ADC1 Channel 5 (non-blocking)
        int raw = adc1_get_raw(ADC1_CHANNEL_5);
        
        // Update position atomically
        data->positions[0] = (float)raw;
        
        // Print every 50 reads (~500ms at 10ms interval)
        if (read_count++ % 50 == 0) {
            printf("ADC_CH5: %d -> position: %.1f\n", raw, data->positions[0]);
        }
        
        // Sleep 10ms between reads (100 Hz sampling)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    vTaskDelete(NULL);
}