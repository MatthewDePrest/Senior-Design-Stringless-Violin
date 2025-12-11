#include "distance_calc.h"
#include <math.h>

float calculate_distance(const ImuPacket *a, const ImuPacket *b) {
    float dx = a->imu.ax - b->imu.ax;
    float dy = a->imu.ay - b->imu.ay;
    float dz = a->imu.az - b->imu.az;
    printf("A device = [%.2f, %.2f, %.2f]\n", a->imu.ax, a->imu.ay, a->imu.az);
    printf("B device = [%.2f, %.2f, %.2f]\n", b->imu.ax, b->imu.ay, b->imu.az);
    return sqrtf(dx*dx + dy*dy + dz*dz);
}