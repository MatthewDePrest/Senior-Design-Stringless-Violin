#include "main.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"  // esp_rom_delay_us

// Define a local tag for logging
static const char *TAG = "touchSensorTask";

static inline int median3(int a, int b, int c) {
    if (a > b) { int t=a; a=b; b=t; }
    if (b > c) { int t=b; b=c; c=t; }
    if (a > b) { int t=a; a=b; b=t; }
    return b;
}

void adc_init(void) {
    adc1_config_width(ADC_WIDTH_BIT_12);

    // Use non-deprecated attenuation (ADC_ATTEN_DB_12 == old 11 dB alias)
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12); // GPIO6
    // adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12); // GPIO5
    // adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_12); // GPIO4

    // set as analog
    gpio_config_t io_conf_ana = {};
    io_conf_ana.pin_bit_mask = 1ULL << 6;
    io_conf_ana.mode = GPIO_MODE_DISABLE;
    io_conf_ana.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf_ana.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf_ana);

    // Set pins 5 and 4 to digital
    gpio_config_t io_conf_dig = {};
    io_conf_dig.pin_bit_mask = (1ULL << 5) | (1ULL << 4) | (1ULL << 18) | (1ULL << 8);
    io_conf_dig.mode = GPIO_MODE_INPUT;
    io_conf_dig.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf_dig.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_config(&io_conf_dig);

    ESP_LOGI(TAG, "ADC initialized (CH5=GPIO6, CH4=GPIO5, CH3=GPIO4, 12-bit, 12dB)");
}

void touchSensor_task(void *pvParameters) {
    allData *data = (allData *)pvParameters;

    adc_init();
    ESP_LOGI(TAG, "Touch sensor task started");
    vTaskDelay(pdMS_TO_TICKS(100));
    int read_count = 0;
    while (!data->end) {
        int raw = adc1_get_raw(ADC1_CHANNEL_5);
        // int raw2 = adc1_get_raw(ADC1_CHANNEL_4);
        int stringE = gpio_get_level(8);
        int stringA = gpio_get_level(18);
        int stringD = gpio_get_level(4);
        int stringG = gpio_get_level(5);

        data->positions[0] = (float)raw;
        data->positions[1] = (float)raw;
        data->positions[2] = (float)raw;
        data->positions[3] = (float)raw;
        data->pressures[0] = stringG ? 1023 : 0;
        data->pressures[1] = stringD ? 1023 : 0;
        data->pressures[2] = stringA ? 1023 : 0;
        data->pressures[3] = stringE ? 1023 : 0;
        //data->pressures[3] = raw3;
        if (read_count++ % 50 == 0) {
            //printf("ADC raw=%4d, ADC raw2=%4d,ADC raw3=%4d\n", raw, stringE, stringA);
            printf("\rraw=%4d  E=%d  A=%d  D=%d  G=%d    \x1b[0K",
               raw, stringE, stringA, stringD, stringG);
            fflush(stdout);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(NULL);
}