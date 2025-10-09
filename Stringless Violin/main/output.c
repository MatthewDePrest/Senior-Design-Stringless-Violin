#define _USE_MATH_DEFINES
#include <math.h>
#include "main.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void output(void *pvParameters) {
    allData *data = (allData *)pvParameters;

    printf("[Core %d] Note conversion running...\n", xPortGetCoreID());

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

        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}