#ifndef MAIN_H
#define MAIN_H

#include "mpu6050.h"
#include "esp_now_reciever.h"  // ADD THIS to get ImuPacket and MPU6050_Data
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>

// Note conversion parameters (used in noteConversion.c)
extern float STRING_LEN;        // Length of the strings on the violin's neck, in mm
extern float BASE_FREQUENCY;    // Frequency of the lowest note playable, in Hz
// typedef struct {
//     MPU6050_Data imu;
// } ImuPacket;

typedef struct {
    int pressures[4];
    float positions[4];
    float stringsFreqs[4];
    // bow speed stored as milli-units in an atomic int to ensure lock-free visibility
    _Atomic int32_t bowSpeed_milli;
    int end;
} allData;

void noteConversion(allData *data);
void output(void *pvParameters);
void accelerometer(allData *data);
void pressureSensor(allData *data);
void touchSensor(allData *data);

#endif