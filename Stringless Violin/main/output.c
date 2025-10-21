// Simple waveform generator for testing and future codec hookup.
// Generates mono signed 16-bit PCM by mixing up to 4 sine oscillators.

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include "main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "audio_driver.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Output task: generates audio buffer and hands to audio driver.
void output(void *pvParameters) {
    allData *data = (allData *)pvParameters;

    const int sample_rate = 16000; // 16 kHz is fine for testing
    const int buffer_ms = 20; // buffer duration in ms
    const int buf_samples = (sample_rate * buffer_ms) / 1000;

    // Per-string phase accumulators
    double phases[4] = {0};

    // Initialize audio driver (stub). Real driver will be wired later.
    audio_driver_init(sample_rate);

    int16_t *pcm = malloc(sizeof(int16_t) * buf_samples);
    if (!pcm) {
        printf("output: failed to allocate pcm buffer\n");
        vTaskDelete(NULL);
        return;
    }

    while (!data->end) {
        // Update target frequencies
        noteConversion(data);

        // Debug: print inputs and derived frequencies occasionally (once per second)
        static int debug_tick = 0;
        debug_tick++;
        if (debug_tick >= (1000 / buffer_ms)) {
            debug_tick = 0;
            printf("pos: [%.1f, %.1f, %.1f, %.1f]  freq: [%.1f, %.1f, %.1f, %.1f]\n",
                   data->positions[0], data->positions[1], data->positions[2], data->positions[3],
                   data->stringsFreqs[0], data->stringsFreqs[1], data->stringsFreqs[2], data->stringsFreqs[3]);
            printf("press: [%d, %d, %d, %d] bow: %.1f\n\n",
                   data->pressures[0], data->pressures[1], data->pressures[2], data->pressures[3],
                   data->bowSpeed);
        }

        // Simple per-buffer generation
        for (int n = 0; n < buf_samples; n++) {
            double sample = 0.0;

            // Mix four strings
            for (int s = 0; s < 4; s++) {
                // Use pressure threshold as gate and normalize amplitude
                double gate = (data->pressures[s] > 10) ? 1.0 : 0.0;
                if (gate > 0) {
                    double freq = data->stringsFreqs[s];
                    // amplitude influenced by bowSpeed and pressure
                    double amplitude = (data->bowSpeed / 100.0) * (data->pressures[s] / 1023.0);
                    double incr = (2.0 * M_PI * freq) / (double)sample_rate;
                    sample += amplitude * sin(phases[s]) * gate;
                    phases[s] += incr;
                    if (phases[s] > (2.0 * M_PI)) phases[s] -= (2.0 * M_PI);
                }
            }

            // Simple soft clipping and scale to int16
            if (sample > 1.0) sample = 1.0;
            if (sample < -1.0) sample = -1.0;
            pcm[n] = (int16_t)(sample * 30000.0);
        }

        // Send to audio driver (stub). In future this will write to I2S/codec.
        // For diagnostics, print first few pcm samples occasionally
        if (debug_tick == 0) {
            int toprint = buf_samples < 8 ? buf_samples : 8;
            printf("pcm[0..%d]:", toprint - 1);                         //TODO: figure out why this prints all zeros
            for (int i = 0; i < toprint; ++i) printf(" %d", pcm[i]);
            printf("\n");
        }

        audio_driver_write(pcm, buf_samples);

        // Small delay -- buffer_ms worth of time
        vTaskDelay(pdMS_TO_TICKS(buffer_ms));
    }

    free(pcm);
    audio_driver_deinit();
    vTaskDelete(NULL);
}