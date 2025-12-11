// language: c
// filepath: main/touchSensor.c
//This is for the hand positions on the neck of the violin and holding down the strings
#include "main.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "touchSensor";

void adc_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);
    // Configure channel for your potentiometer (check which pin you connected it to)
    // GPIO4 → ADC1_CHANNEL_3
    // GPIO5 → ADC1_CHANNEL_4
    // Adjust channel if your pot is on a different pin
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
}

void touchSensor(allData *data) {
    static bool first_call = true;
    
    if (first_call) {
        adc_init();
        first_call = false;
    }
    
    // Read ADC1_CHANNEL_3 (GPIO4)
    int raw = adc1_get_raw(ADC1_CHANNEL_3);
    
    // Update position for string 0 (G string)
    data->positions[0] = (float)raw;
    
    // For now, set other strings to mid-range so they play
    // (In full implementation, you'd have pots for each string)
    data->positions[1] = (float)raw * 0.8f;  // D string slightly lower
    data->positions[2] = (float)raw * 0.6f;  // A string mid-range
    data->positions[3] = (float)raw * 0.4f;  // E string higher
    
    static int debug_count = 0;
    if (debug_count++ % 50 == 0) {
        printf("touchSensor: raw=%d → positions=[%.1f, %.1f, %.1f, %.1f]\n", 
               raw, data->positions[0], data->positions[1], data->positions[2], data->positions[3]);
    }
}