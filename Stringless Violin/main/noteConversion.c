#include "main.h"
#include <math.h>

// Define the global variables declared as extern in main.h
float STRING_LEN = 330.0f;        // Length of the strings on the violin's neck, in mm
float BASE_FREQUENCY = 196.0f;    // Frequency of the lowest note playable, in Hz

float base[4];

static void defaultFrequency() {
    // base[0] = 196.0;
    // base[1] = 293.0;
    // base[2] = 440.0;
    // base[3] = 659.0;
    base[0] = 131.0;
    base[1] = 196.0;
    base[2] = 293.0;
    base[3] = 440.0;
    return;
}

__attribute__((unused)) static void read_frequencies() {
    FILE *file = fopen("../App/violin_frequencies.txt", "r");
    if (file == NULL) {
        perror("Failed to open frequencies file");
        defaultFrequency();
        return;
    }

    for (int i = 0; i < 4; i++) {
        if (!(fscanf(file, "%f\n", &base[i]))) {
            fprintf(stderr, "Failed to read frequency of string %d\n", i);
            fclose(file);
            defaultFrequency();
            return;
        }
    }

    fclose(file);
    return;
}

void noteConversion(allData *data) {
    defaultFrequency();
    // read_frequencies();
    
    // Physical violin model: frequency is inversely proportional to string length
    // f = f_open * (L_total / L_remaining)
    // When ADC = max (open string), play at base frequency (lowest)
    // When ADC = min (fully pressed), highest frequency
    
    const float MIN_PLAYABLE_LENGTH = 30.0f;   // mm - minimum length that can vibrate (avoid extreme highs)
    const float SENSOR_LENGTH_MM = 200.0f;     // mm - touch sensor physical length
    const float OPEN_TAIL_MM = 20.0f;          // mm - bottom segment treated as open string
    const float ADC_OPEN_THRESHOLD = (OPEN_TAIL_MM / SENSOR_LENGTH_MM) * 4095.0f; // ~10% from bottom
    
    for (int s = 0; s < 4; ++s) {
        float adc_pos = data->positions[s];
        
        // Clamp ADC range to 0-4095
        if (adc_pos < 0.0f) adc_pos = 0.0f;
        if (adc_pos > 4095.0f) adc_pos = 4095.0f;
        
        // If finger is in the bottom 20mm (lowest ADC), snap directly to open-string frequency
        if (adc_pos <= ADC_OPEN_THRESHOLD) {
            data->stringsFreqs[s] = base[s];
            continue;
        }

        // Linearize the ADC response by taking square root to counteract exponential pot resistance
        // This stretches out the compressed upper range and compresses the lower range
        float normalized_adc = adc_pos / 4095.0f;
        float linearized_adc = sqrtf(normalized_adc);  // sqrt inverts exponential compression
        
        // Map ADC to remaining string length (open string reads high ADC)
        // adc_pos = 4095 -> full length, adc_pos = 0 -> shortest
        float remaining_length = STRING_LEN * linearized_adc;

        
        // Clamp to minimum playable length to avoid extreme frequencies
        if (remaining_length < MIN_PLAYABLE_LENGTH) {
            remaining_length = MIN_PLAYABLE_LENGTH;
        }
        
        // Apply inverse frequency relationship (physics of vibrating strings)
        // Higher frequency = shorter string
        float frequency_ratio = STRING_LEN / remaining_length;
        data->stringsFreqs[s] = base[s] * frequency_ratio;
    }
}