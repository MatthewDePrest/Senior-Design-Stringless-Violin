#include "../Headers/main.h"
#include <math.h>

void noteConversion(allData *data) {
    float base[4] = {BASE_FREQUENCY, 1.5 * BASE_FREQUENCY, 2.25 * BASE_FREQUENCY, 3.375 * BASE_FREQUENCY};

    for (int i = 0; i < 4; i++) {
        data->strings[i] = base[i] * (STRING_LEN / (STRING_LEN - data->positions[i]));
    }
}