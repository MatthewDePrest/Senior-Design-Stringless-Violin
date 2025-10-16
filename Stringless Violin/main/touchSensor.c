//This is for the hand positions on the neck of the violin and holding down the strings
#include "main.h"
// adc_reader.c
#include "driver/adc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_adc/adc_oneshot.h"

#include "driver/adc.h"

 

adc_oneshot_unit_handle_t adc;

void adc_init(void) {

    adc_oneshot_unit_init_cfg_t unit_cfg = { .unit_id = ADC_UNIT_1 };

    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);

    // ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc));

 

    adc_oneshot_chan_cfg_t ch_cfg = { .bitwidth = ADC_BITWIDTH_DEFAULT,

                                      .atten = ADC_ATTEN_DB_11 };

    // ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, ADC_CHANNEL_2, &ch_cfg));

}

void touchSensor(allData *data) {

    int raw = 0;
    int count = 0;
    while(count < 50) {
        adc_init();
        raw = adc1_get_raw(ADC1_CHANNEL_5);

        // adc2_get_raw(ADC_CHANNEL_2, ADC_WIDTH_BIT_12, &raw);

        // ESP_ERROR_CHECK(adc_oneshot_read(adc,ADC_CHANNEL_2, &raw));
        vTaskDelay(pdMS_TO_TICKS(10));
        printf("raw: %d\n", raw); // raw ADC code (no calibration)
    }
}