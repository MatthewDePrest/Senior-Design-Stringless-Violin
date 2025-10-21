#include "audio_driver.h"
#include <stdio.h>

static int g_sample_rate = 0;

esp_err_t audio_driver_init(int sample_rate) {
    // Placeholder: configure I2S / codec when hardware is available.
    g_sample_rate = sample_rate;
    printf("audio_driver: init at %d Hz (stub)\n", sample_rate);
    return 0; // ESP_OK
}

size_t audio_driver_write(const int16_t *samples, size_t sample_count) {
    // Placeholder: in real driver this would push to DMA buffer / I2S.
    // For now just pretend we consumed all samples.
    (void)samples;
    return sample_count;
}

void audio_driver_deinit(void) {
    printf("audio_driver: deinit (stub)\n");
    g_sample_rate = 0;
}
