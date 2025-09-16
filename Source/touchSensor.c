//This is for the hand positions on the neck of the violin and holding down the strings
#include "../Headers/main.h"

void touchSensor(allData *data) {
    data->positions[0] = 50; // Example position values in mm
    data->positions[1] = 60;
    data->positions[2] = 70;
    data->positions[3] = 80;
}