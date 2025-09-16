#include "../Headers/main.h"

void noteConversion(allData *data) {
    static float STRING_LEN = 200; //mm

    float opens[4] = {196.00, 293.66, 440.00, 659.25}; //calculate this later by base frequency

    data->strings[0] = opens[0] * STRING_LEN / (STRING_LEN - data->positions[0]);
    data->strings[1] = opens[1] * STRING_LEN / (STRING_LEN - data->positions[1]);
    data->strings[2] = opens[2] * STRING_LEN / (STRING_LEN - data->positions[2]);
    data->strings[3] = opens[3] * STRING_LEN / (STRING_LEN - data->positions[3]);

}