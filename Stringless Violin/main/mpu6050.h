#ifndef MPU6050_H
#define MPU6050_H

#include <stdint.h>

typedef struct {
    float ax, ay, az;  // acceleration (g)
    float gx, gy, gz;  // angular velocity (°/s)
    float temp;        // temperature (°C)
} MPU6050_Data;

void mpu6050_init(void);
void mpu6050_read(MPU6050_Data *data);
void mpu6050_deinit(void);

#endif // MPU6050_H