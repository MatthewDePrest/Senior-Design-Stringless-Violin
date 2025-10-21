#include "main.h"
#include <math.h>

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

static float toMM(float positions) {
    int length;
    length = (positions / 1900) * 300.0; // Scale to 0-300 mm
    return length;
}

void noteConversion(allData *data) {
    //read_frequencies();
    defaultFrequency();

    data->stringsFreqs[0] = base[0] * (STRING_LEN / (STRING_LEN - toMM(data->positions[0])));
    data->stringsFreqs[1] = base[1] * (STRING_LEN / (STRING_LEN - toMM(data->positions[1])));
    data->stringsFreqs[2] = base[2] * (STRING_LEN / (STRING_LEN - toMM(data->positions[2])));
    data->stringsFreqs[3] = base[3] * (STRING_LEN / (STRING_LEN - toMM(data->positions[3])));
}