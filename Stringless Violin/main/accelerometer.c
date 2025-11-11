//This code represents and detects the speed the bow goes across the violins cords
#include "main.h"
#include <inttypes.h>

void accelerometer(allData *data) {
    // Store as milli-units (100.0 -> 100000)
    atomic_store(&data->bowSpeed_milli, (int32_t)(100.0f * 1000.0f)); // mm/s
    // Debug: confirm store
    int32_t v = atomic_load(&data->bowSpeed_milli);
    printf("accelerometer: stored bowSpeed_milli=%ld\n", (long)v);
}