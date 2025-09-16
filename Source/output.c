#include <math.h>
#include "../Headers/main.h"

void output(allData *data) {
    noteConversion(data);
    float total = 0;
    float amplitude = data->bowSpeed;

    float t = 0.01; //time variable, should be incremented in real implementation

    for(int i = 0; i < 4; i++){
        if (data->pressures[i] > 10){
            total += (amplitude * data->pressures[i]) * sin(2 * 3.1415926535897932384 * data->strings[i] * t);
        }
    }
}