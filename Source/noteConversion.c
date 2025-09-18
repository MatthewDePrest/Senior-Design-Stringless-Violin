#include "../Headers/main.h"
#include <math.h>

void noteConversion(allData *data) {
    float base[4] = {BASE_FREQUENCY, 1.5 * BASE_FREQUENCY, 2.25 * BASE_FREQUENCY, 3.375 * BASE_FREQUENCY};

    // data->strings[0] = base[0] * STRING_LEN / (STRING_LEN - data->positions[0]);
    // data->strings[1] = base[1] * STRING_LEN / (STRING_LEN - data->positions[1]);
    // data->strings[2] = base[2] * STRING_LEN / (STRING_LEN - data->positions[2]);
    // data->strings[3] = base[3] * STRING_LEN / (STRING_LEN - data->positions[3]);
    for (int i = 0; i < 4; i++) {
        data->strings[i] = base[i] * pow(1.5, (data->positions[i] / STRING_LEN));
    }
}