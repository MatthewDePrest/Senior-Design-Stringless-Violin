#ifndef MPU6050_H
#define MPU6050_H

typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float temp;
} MPU6050_Data;

void mpu6050_init(void);
void mpu6050_read(MPU6050_Data *data);

#endif