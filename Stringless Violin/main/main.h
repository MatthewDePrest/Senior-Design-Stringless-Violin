#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdatomic.h>

static float STRING_LEN = 330; // Length of the strings on the violin's neck, in mm
static float BASE_FREQUENCY = 196; // Frequency of the lowest note playable, in Hz

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