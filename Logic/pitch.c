#include <math.h>

int calculatePitch(int stringPressed, double positionPressed, int minFreq) {
    if (stringPressed < 1 || stringPressed > 4) return -1;
    if (positionPressed < 0 || positionPressed > 1) return -1;
    if (minFreq < 0) return -1;
    double baseFreq = minFreq * pow(1.5, stringPressed - 1);
    double freq = baseFreq * pow(1.5, positionPressed);
    return (int) freq;
}
