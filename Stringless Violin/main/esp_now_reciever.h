#ifndef ESP_NOW_RECIEVER_H
#define ESP_NOW_RECIEVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float temp;
} MPU6050_Data;

typedef struct {
    MPU6050_Data imu;
} ImuPacket;

void esp_now_receiver_init(void);
void update_local_imu(void);
ImuPacket* get_remote_imu(void);
ImuPacket* get_local_imu(void);

#endif