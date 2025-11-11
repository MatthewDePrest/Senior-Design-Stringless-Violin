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
    // Read a single sample for initialization. Long-running sampling
    // belongs in a dedicated task rather than a blocking function.
    static bool first_call = true;
    int raw = 0;
    
    if (first_call) {
        adc_init();
        first_call = false;
    }
    
    raw = adc1_get_raw(ADC1_CHANNEL_3);
    data->positions[0] = (float)raw;
    
    // Debug: print occasionally to verify readings
    static int debug_count = 0;
    if (debug_count++ % 100 == 0) {
        printf("touchSensor: raw=%d, pos[0]=%.1f\n", raw, data->positions[0]);
    }
}