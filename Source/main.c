#include "../Headers/main.h"


int main(int argc, char const *argv[])
{
    allData data;
    pressureSensor(&data);  //data.pressures = {20, 0, 0, 0};
    touchSensor(&data);      //data.positions = {50, 60, 70, 80};
    accelerometer(&data);    //data.bowSpeed = 100;

    output(&data);

    // Print all values saved in allData
    printf("Pressures: [%d, %d, %d, %d]\n", data.pressures[0], data.pressures[1], data.pressures[2], data.pressures[3]);
    printf("Positions: [%.1f, %.1f, %.1f, %.1f]\n", data.positions[0], data.positions[1], data.positions[2], data.positions[3]);
    printf("Bow Speed: %.1f\n", data.bowSpeed);
    printf("\nOutputs: [%.1f Hz, %.1f Hz, %.1f Hz, %.1f Hz]\n\n", data.stringsFreqs[0], data.stringsFreqs[1], data.stringsFreqs[2], data.stringsFreqs[3]);

    return 0;
}
