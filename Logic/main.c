#include "pitch.c"
#include "stdio.h"

int main(int argc, char const *argv[])
{
    int stringPressed = 1;
    double positionPressed = 0.55;
    int baseFreq = 196;
    int freq = calculatePitch(stringPressed, positionPressed, baseFreq);
    printf("%d\n", freq);
    return 0;
}
