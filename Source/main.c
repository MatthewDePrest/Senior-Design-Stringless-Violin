#include <stdio.h>
#include "pitch.c"
#include "../Headers/main.h"

int main(int argc, char const *argv[])
{
    allData* data;
    data->pressures[0] = 10;
    data->pressures[0] = 0;
    data->pressures[0] = 6;
    data->pressures[0] = 14;
    data->positions[0] = 0.5;
    data->positions[0] = 0.5;
    data->positions[0] = 0.5;
    data->positions[0] = 0.5;
    data->bowSpeed = -6;
    data->lowestFreq = 120;

    calculatePitch(data);
    printf("%.1f\n", data->freq);
    return 0;
}
