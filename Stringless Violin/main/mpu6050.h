#ifndef MPU6050_H
#define MPU6050_H

#include "esp_now_reciever.h"  // Get MPU6050_Data from here

void mpu6050_init(void);
void mpu6050_read(MPU6050_Data *data);

#endif