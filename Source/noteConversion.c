#include "../Headers/main.h"
#include <math.h>

float base[4];

void defaultFrequency() {
    base[0] = 196.0;
    base[1] = 293.0;
    base[2] = 440.0;
    base[3] = 659.0;
    return;
}

void read_frequencies() {
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
    read_frequencies();

    for (int i = 0; i < 4; i++) {
        data->strings[i] = base[i] * (STRING_LEN / (STRING_LEN - data->positions[i]));
    }
}