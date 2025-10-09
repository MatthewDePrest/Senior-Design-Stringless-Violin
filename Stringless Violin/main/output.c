#define _USE_MATH_DEFINES
#include <math.h>
#include "main.h"

void output(allData *data) {
    while(!data->end) {
        noteConversion(data);
        float total = 0;
        float amplitude = data->bowSpeed;

        float t = 0.01; //time variable, should be incremented in real implementation

        for(int i = 0; i < 4; i++) {
            if (data->pressures[i] > 10) {
                total += (amplitude * data->pressures[i]) * (2 * M_PI * data->stringsFreqs[i] * t); // TODO: include sin
            }
        }
    }
}