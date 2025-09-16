#ifndef MAIN_H
#define MAIN_H

typedef struct {
    int pressures[4];
    float positions[4];
    float strings[4];
    float bowSpeed;
    float freq;
} allData;

void noteConversion(allData *data);
void output(allData *data);
void accelerometer(allData *data);
void pressureSensor(allData *data);
void touchSensor(allData *data);

#endif