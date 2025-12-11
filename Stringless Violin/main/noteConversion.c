#include "main.h"
#include <math.h>

// Define the global variables declared as extern in main.h
float STRING_LEN = 330.0f;        // Length of the strings on the violin's neck, in mm
float BASE_FREQUENCY = 196.0f;    // Frequency of the lowest note playable, in Hz

float base[4];

static void defaultFrequency() {
    base[0] = 196.0;
    base[1] = 293.0;
    base[2] = 440.0;
    base[3] = 659.0;
    return;
}

static void read_frequencies() {
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
    //defaultFrequency();
    read_frequencies();
    // Linear map ADC position (0..4095) to frequency within each string's range
    const float max_freq[4] = { 293.66f, 440.00f, 659.25f, 987.77f };

    for (int s = 0; s < 4; ++s) {
        float pos = data->positions[s];
        
        // Clamp ADC range to 0-4095
        if (pos < 0.0f) pos = 0.0f;
        if (pos > 4095.0f) pos = 4095.0f;  // FIX: was >= 4000.0f pos = 0.0f
        
        float frac = pos / 4095.0f;  // 0..1
        float fmin = base[s];
        float fmax = max_freq[s];
        data->stringsFreqs[s] = fmin + frac * (fmax - fmin);
    }
}