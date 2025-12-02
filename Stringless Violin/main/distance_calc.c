#include "distance_calc.h"
#include <math.h>

float calculate_distance(const ImuPacket *a, const ImuPacket *b) {
    float dx = a->imu.ax - b->imu.ax;
    float dy = a->imu.ay - b->imu.ay;
    float dz = a->imu.az - b->imu.az;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}