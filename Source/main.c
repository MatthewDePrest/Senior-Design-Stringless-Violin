#include "pitch.c"
#include "stdio.h"

int main(int argc, char const *argv[])
{
    int stringPressed = 1;
    double positionPressed = 0.55;
    float baseFreq = 196;
    float freq = calculatePitch(stringPressed, positionPressed, baseFreq);
    printf("%.1f\n", freq);
    return 0;
}
