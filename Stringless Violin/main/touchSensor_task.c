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
    
    // Configure only ADC1_CHANNEL_5 (GPIO6) Or pin 6 
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_12);

    // Configure GPIO6 as input (no pull-ups for analog)
    gpio_config_t io_conf = {};
    io_conf.pin_bit_mask = (1ULL << 6);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
    // gpio_config_t io_conf = {};
    // io_conf.pin_bit_mask = (1ULL << 5);
    // io_conf.mode = GPIO_MODE_INPUT;
    // io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    // gpio_config(&io_conf);
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
        int raw2 = adc1_get_raw(ADC1_CHANNEL_4);
        int raw3 = adc1_get_raw(ADC1_CHANNEL_3);

        
        // Map ADC value to 0-4095 range using calibration
        float position = 2048.0f;
        
        
        // Update only string 0
        data->positions[0] = raw;
        data->pressures[0] = raw2 * 5;
        data->pressures[3] = raw3 * 100;
        // Print every 50 reads (~500ms at 10ms interval)
        if (read_count++ % 50 == 0) {
            printf("ADC raw=%4d → position=%.0f, ADC raw2=%4d\n", raw, position, raw2);
        
        }
        
        // Sleep 10ms between reads (100 Hz sampling)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    vTaskDelete(NULL);
}