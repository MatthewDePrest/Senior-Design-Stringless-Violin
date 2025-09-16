#include <stdio.h>
#include "pitch.c"
#include "../Headers/main.h"

int main(int argc, char const *argv[])
{
    allData data;
    data.pressures[0] = 10;
    data.pressures[1] = 0;
    data.pressures[2] = 6;
    data.pressures[3] = 14;
    data.positions[0] = 0;
    data.positions[1] = 0;
    data.positions[2] = 0;
    data.positions[3] = 0;
    data.bowSpeed = -6;
    data.lowestFreq = 120;
    data.freq = 200;

    calculatePitch(&data);
    printf("%.1f\n", data.freq);
    return 0;
}
