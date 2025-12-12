#ifndef ESP_NOW_RECIEVER_H
#define ESP_NOW_RECIEVER_H

#include <stdint.h>
#include <stdbool.h>

// Forward declare shared data struct from main.h to avoid circular include
typedef struct allData allData;

typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float temp;
} MPU6050_Data;

typedef struct {
    MPU6050_Data imu;
} ImuPacket;

typedef struct {
    int pressures[4];
    float positions[4];
} NeckSensorPacket;

void esp_now_receiver_init(void);
void update_local_imu(void);
ImuPacket* get_remote_imu(void);
ImuPacket* get_local_imu(void);
NeckSensorPacket* get_neck_sensors(void);
void esp_now_set_data_ptr(allData *ptr);

#endif