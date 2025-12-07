#include "mpu6050.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include <math.h>

static const char *TAG = "MPU6050";

#define MPU6050_ADDR 0x68
#define MPU6050_I2C_PORT I2C_NUM_1
#define MPU6050_SCL_PIN 12    // not GPIO4/6
#define MPU6050_SDA_PIN 11    // not GPIO4/6
// ...existing code...

#define MPU6050_REG_WHOAMI 0x75
#define MPU6050_REG_PWR_MGMT_1 0x6B
#define MPU6050_REG_ACCEL_XOUT_H 0x3B

static bool mpu6050_initialized = false;

static esp_err_t mpu6050_write_reg(uint8_t reg, uint8_t value) {
    uint8_t write_buf[2] = {reg, value};
    esp_err_t ret = i2c_master_write_to_device(MPU6050_I2C_PORT, MPU6050_ADDR, write_buf, sizeof(write_buf), pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Write reg 0x%02X failed: 0x%X", reg, ret);
    }
    return ret;
}

static esp_err_t mpu6050_read_reg(uint8_t reg, uint8_t *data, uint8_t len) {
    esp_err_t ret = i2c_master_write_read_device(MPU6050_I2C_PORT, MPU6050_ADDR, &reg, 1, data, len, pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read reg 0x%02X failed: 0x%X", reg, ret);
    }
    return ret;
}

void mpu6050_init(void) {
    if (mpu6050_initialized) return;

    ESP_LOGI(TAG, "Initializing I2C on pins SCL=%d, SDA=%d", MPU6050_SCL_PIN, MPU6050_SDA_PIN);

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = MPU6050_SDA_PIN,
        .scl_io_num = MPU6050_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    esp_err_t ret = i2c_param_config(MPU6050_I2C_PORT, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: 0x%X", ret);
        return;
    }

    ret = i2c_driver_install(MPU6050_I2C_PORT, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2c_driver_install failed: 0x%X", ret);
        return;
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    // Check WHO_AM_I
    uint8_t whoami = 0;
    mpu6050_read_reg(MPU6050_REG_WHOAMI, &whoami, 1);
    
    ESP_LOGI(TAG, "MPU6050 WHO_AM_I = 0x%02X (expect 0x68)", whoami);
    
    if (whoami != 0x68) {
        ESP_LOGE(TAG, "MPU6050 not found!");
        return;
    }

    // Wake up MPU6050 (clear sleep bit)
    mpu6050_write_reg(MPU6050_REG_PWR_MGMT_1, 0x00);
    vTaskDelay(pdMS_TO_TICKS(100));

    mpu6050_initialized = true;
    ESP_LOGI(TAG, "MPU6050 initialized successfully");
}

void mpu6050_read(MPU6050_Data *data) {
    if (!mpu6050_initialized || !data) {
        if (!mpu6050_initialized) {
            //ESP_LOGW(TAG, "MPU6050 not initialized");
        }
        return;
    }

    uint8_t buffer[14];
    esp_err_t ret = mpu6050_read_reg(MPU6050_REG_ACCEL_XOUT_H, buffer, 14);
    
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read sensor data");
        return;
    }

    // Accel (3 axes): bytes 0-5
    int16_t ax = (int16_t)((buffer[0] << 8) | buffer[1]);
    int16_t ay = (int16_t)((buffer[2] << 8) | buffer[3]);
    int16_t az = (int16_t)((buffer[4] << 8) | buffer[5]);

    // Temp: bytes 6-7
    int16_t temp_raw = (int16_t)((buffer[6] << 8) | buffer[7]);

    // Gyro (3 axes): bytes 8-13
    int16_t gx = (int16_t)((buffer[8] << 8) | buffer[9]);
    int16_t gy = (int16_t)((buffer[10] << 8) | buffer[11]);
    int16_t gz = (int16_t)((buffer[12] << 8) | buffer[13]);

    // Convert to float
    data->ax = ax / 16384.0f;
    data->ay = ay / 16384.0f;
    data->az = az / 16384.0f;

    data->gx = gx / 131.0f;
    data->gy = gy / 131.0f;
    data->gz = gz / 131.0f;

    data->temp = (temp_raw / 340.0f) + 36.53f;
}

void mpu6050_deinit(void) {
    if (mpu6050_initialized) {
        i2c_driver_delete(MPU6050_I2C_PORT);
        mpu6050_initialized = false;
        ESP_LOGI(TAG, "MPU6050 deinitialized");
    }
}