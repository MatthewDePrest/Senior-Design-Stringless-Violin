#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

// Initialize the audio output system at the requested sample rate.
// This is a thin abstraction: the real codec/I2S implementation will be
// provided later when the hardware arrives.
esp_err_t audio_driver_init(int sample_rate);

// Write signed 32-bit PCM mono samples. Returns number of samples consumed.
size_t audio_driver_write(const int32_t *samples, size_t sample_count);

// Deinitialize/free resources.
void audio_driver_deinit(void);

#endif // AUDIO_DRIVER_H
