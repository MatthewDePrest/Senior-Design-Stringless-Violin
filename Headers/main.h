#ifndef MAIN_H
#define MAIN_H

#include <stdlib.h>
#include <stdio.h>

static float STRING_LEN = 330; // Length of the strings on the violin's neck, in mm
static float BASE_FREQUENCY = 196; // Frequency of the lowest note playable, in Hz

typedef struct {
    int pressures[4];
    float positions[4];
    float stringsFreqs[4];
    float bowSpeed;
} allData;

void noteConversion(allData *data);
void output(allData *data);
void accelerometer(allData *data);
void pressureSensor(allData *data);
void touchSensor(allData *data);

#endif