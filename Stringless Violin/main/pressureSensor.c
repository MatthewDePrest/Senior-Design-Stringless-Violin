//This file is for the pressure sensor of the bow pushing down on the strings
#include "main.h"

void pressureSensor(allData *data) {
    data->pressures[0] = 1000; // Example pressure values
    data->pressures[1] = 0;
    data->pressures[2] = 0;
    data->pressures[3] = 0;
}