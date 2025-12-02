# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
idf_component_register(SRCS
    "accelerometer.c"
    "audio_driver.c"
    "esp_now_receiver.c"
    "main.c"
    "mpu6050.c"
    "noteConversion.c"
    "output.c"
    "pressureSensor.c"
    "touchSensor.c"
    "distance_calc.c"
    INCLUDE_DIRS ".")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(Stringless Violin)