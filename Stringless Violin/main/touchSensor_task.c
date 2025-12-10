#include "main.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "touchSensor";

void adc_init(void) {
    // Initialize ADC1 with 12-bit resolution
    adc1_config_width(ADC_WIDTH_BIT_12);
    
    // Configure only ADC1_CHANNEL_5 (GPIO6)
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);
    
    // Configure GPIO6 as input (no pull-ups for analog)
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << 6);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "ADC initialized for single channel (GPIO6)");
}

void touchSensor_task(void *pvParameters) {
    allData *data = (allData *)pvParameters;
    
    // Initialize ADC once at startup
    adc_init();
    ESP_LOGI(TAG, "Touch sensor task started");
    
    vTaskDelay(pdMS_TO_TICKS(100));  // Let ADC settle
    
    // Calibration variables
    static int calibration_count = 0;
    static int calibration_min = 4095;
    static int calibration_max = 0;
    
    int read_count = 0;
    
    while (!data->end) {
        // Read only ADC1_CHANNEL_5 (GPIO6)
        int raw = adc1_get_raw(ADC1_CHANNEL_5);
        
        // Calibration: track min/max values for first 5 seconds
        if (calibration_count < 500) {
            if (raw < calibration_min) calibration_min = raw;
            if (raw > calibration_max) calibration_max = raw;
            
            calibration_count++;
            
            if (calibration_count == 1) {
                ESP_LOGI(TAG, "Calibration starting - move pot through full range...");
            }
            if (calibration_count == 500) {
                ESP_LOGI(TAG, "Calibration complete: min=%d, max=%d", calibration_min, calibration_max);
            }
        }
        
        // Map ADC value to 0-4095 range using calibration
        float position = 2048.0f;
        int range = calibration_max - calibration_min;
        if (range > 0) {
            int clamped = (raw < calibration_min) ? calibration_min :
                          (raw > calibration_max) ? calibration_max : raw;
            position = (float)((clamped - calibration_min) * 4095 / range);
        }
        
        // Update only string 0
        data->positions[0] = position;
        
        // Print every 50 reads (~500ms at 10ms interval)
        if (read_count++ % 50 == 0) {
            printf("ADC raw=%4d → position=%.0f\n", raw, position);
        }
        
        // Sleep 10ms between reads (100 Hz sampling)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    vTaskDelete(NULL);
}