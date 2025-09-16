#include <stdio.h>
#include "../Headers/main.h"

int main(int argc, char const *argv[])
{
    allData data;
    pressureSensor(&data);  //data.pressures = {20, 0, 0, 0};
    touchSensor(&data);      //data.positions = {50, 60, 70, 80};
    accelerometer(&data);    //data.bowSpeed = 100;
    data.freq = 200;

    output(&data);

    printf("%.1f\n", data.freq);

    return 0;
}
