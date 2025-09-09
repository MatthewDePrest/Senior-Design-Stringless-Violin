#include <math.h>
#include "../Headers/main.h"

void calculatePitch(allData *data) {
    double freq = 0;
    if(data->pressures[0] >=10){
        freq += 0.1;
    }
    if(data->pressures[1] >=10){
        freq += 1;
    }
    if(data->pressures[2] >=10){
        freq += 10;
    }
    if(data->pressures[3] >=10){
        freq += 100;
    }
    data->freq = freq;
}
