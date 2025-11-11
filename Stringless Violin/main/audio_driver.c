#include "audio_driver.h"
#include <stdio.h>
#include "driver/i2s.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"

static const char *TAG = "audio_driver";

static int g_sample_rate = 0;
static i2s_port_t s_i2s_port = I2S_NUM_0;

// I2S pin assignment - matching working configuration
#ifndef I2S_PIN_MCLK
#define I2S_PIN_MCLK (GPIO_NUM_1)
#endif
#ifndef I2S_PIN_SCK
#define I2S_PIN_SCK  (GPIO_NUM_17)  // BCLK
#endif
#ifndef I2S_PIN_WS
#define I2S_PIN_WS   (GPIO_NUM_16)  // LRCLK/WS
#endif
#ifndef I2S_PIN_DOUT
#define I2S_PIN_DOUT (GPIO_NUM_15)  // ESP32 TX -> ES8388 DIN
#endif

// I2C pins for ES8388 control
#ifndef ES8388_I2C_SDA
#define ES8388_I2C_SDA (GPIO_NUM_41)
#endif
#ifndef ES8388_I2C_SCL
#define ES8388_I2C_SCL (GPIO_NUM_42)
#endif

// I2C port and 7-bit device address (update if your codec uses different addr)
#ifndef ES8388_I2C_PORT
#define ES8388_I2C_PORT I2C_NUM_0
#endif
#ifndef ES8388_I2C_ADDR
#define ES8388_I2C_ADDR 0x10
#endif

static bool s_es8388_i2c_inited = false;
static uint8_t s_es8388_addr = ES8388_I2C_ADDR;

// I2C helper functions - matching working Arduino code
static bool es8388_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t data[2] = { reg, val };
    esp_err_t ret = i2c_master_write_to_device(ES8388_I2C_PORT, s_es8388_addr, data, sizeof(data), pdMS_TO_TICKS(100));
    if (ret == ESP_OK) {
        // Small delay as in working code
        vTaskDelay(pdMS_TO_TICKS(1));
    } else {
        ESP_LOGE(TAG, "es8388_write_reg: failed to write reg 0x%02x: %d", reg, ret);
    }
    return (ret == ESP_OK);
}

static bool es8388_read_reg(uint8_t reg, uint8_t* val)
{
    esp_err_t ret = i2c_master_write_read_device(ES8388_I2C_PORT, s_es8388_addr, 
                                                  &reg, 1, val, 1, pdMS_TO_TICKS(100));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "es8388_read_reg: failed to read reg 0x%02x: %d", reg, ret);
    }
    return (ret == ESP_OK);
}

// Scan for ES8388 address (can be 0x10 or 0x11)
static uint8_t scan_es8388(void)
{
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t ret = i2c_master_cmd_begin(ES8388_I2C_PORT, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK && (addr == 0x10 || addr == 0x11)) {
            ESP_LOGI(TAG, "Found ES8388 at address 0x%02X", addr);
            return addr;
        }
    }
    return 0;
}

// Initialize I2C peripheral for ES8388 register access.
static esp_err_t es8388_i2c_init(void)
{
    if (s_es8388_i2c_inited) return ESP_OK;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = ES8388_I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = ES8388_I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    esp_err_t ret = i2c_param_config(ES8388_I2C_PORT, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "es8388_i2c: i2c_param_config failed: %d", ret);
        return ret;
    }

    ret = i2c_driver_install(ES8388_I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "es8388_i2c: i2c_driver_install failed: %d", ret);
        return ret;
    }

    s_es8388_i2c_inited = true;
    ESP_LOGI(TAG, "es8388_i2c: initialized on SDA=%d SCL=%d", ES8388_I2C_SDA, ES8388_I2C_SCL);
    return ESP_OK;
}

// ES8388 HP path initialization - from working Arduino code
static esp_err_t es8388_configure_codec(void)
{
    ESP_LOGI(TAG, "Configuring ES8388 codec (HP path)...");
    
    // Reset
    if (!es8388_write_reg(0x00, 0x80)) return ESP_FAIL;
    vTaskDelay(pdMS_TO_TICKS(10));
    if (!es8388_write_reg(0x00, 0x00)) return ESP_FAIL;
    
    // Clock from MCLK
    if (!es8388_write_reg(0x01, 0x50)) return ESP_FAIL;
    
    // I2S slave mode
    if (!es8388_write_reg(0x08, 0x00)) return ESP_FAIL;
    
    // Same LRCK
    if (!es8388_write_reg(0x2B, 0x80)) return ESP_FAIL;
    
    // Digital on
    if (!es8388_write_reg(0x02, 0x00)) return ESP_FAIL;
    
    // I2S format, 24-bit (codec tolerates 32-bit slots)
    if (!es8388_write_reg(0x17, 0x00)) return ESP_FAIL;
    
    // 256*Fs
    if (!es8388_write_reg(0x18, 0x02)) return ESP_FAIL;
    
    // Disable automute (optional)
    es8388_write_reg(0x1C, 0x00);
    
    // DAC 0 dB & unmute
    if (!es8388_write_reg(0x1A, 0x00)) return ESP_FAIL;
    if (!es8388_write_reg(0x1B, 0x00)) return ESP_FAIL;
    if (!es8388_write_reg(0x19, 0x32)) return ESP_FAIL;  // unmute + soft ramp
    
    // Route LDAC/RDAC to HP/LOUT
    if (!es8388_write_reg(0x26, 0x00)) return ESP_FAIL;
    if (!es8388_write_reg(0x27, 0xB8)) return ESP_FAIL;
    if (!es8388_write_reg(0x28, 0x38)) return ESP_FAIL;
    if (!es8388_write_reg(0x29, 0x38)) return ESP_FAIL;
    if (!es8388_write_reg(0x2A, 0xB8)) return ESP_FAIL;
    
    // Analog gains (moderate)
    if (!es8388_write_reg(0x2E, 0x22)) return ESP_FAIL;
    if (!es8388_write_reg(0x2F, 0x22)) return ESP_FAIL;
    if (!es8388_write_reg(0x30, 0x2C)) return ESP_FAIL;
    if (!es8388_write_reg(0x31, 0x2C)) return ESP_FAIL;
    
    // DAC power
    if (!es8388_write_reg(0x0A, 0x00)) return ESP_FAIL;
    
    // HP + LOUT ON
    if (!es8388_write_reg(0x04, 0x3C)) return ESP_FAIL;
    
    // Verify by reading reg 0x00
    uint8_t val = 0;
    bool ok = es8388_read_reg(0x00, &val);
    ESP_LOGI(TAG, "ES8388 reg0x00=0x%02X, init %s", val, ok ? "OK" : "FAIL");
    
    return ok ? ESP_OK : ESP_FAIL;
}

esp_err_t audio_driver_init(int sample_rate) {
    esp_err_t ret;
    g_sample_rate = sample_rate;

    // Configure I2S for master TX, 32-bit samples with APLL (matching working code)
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = g_sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = 256,
        .use_apll = true,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    ret = i2s_driver_install(s_i2s_port, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_driver_install failed: %d", ret);
        return ret;
    }

    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_MCLK,
        .bck_io_num = I2S_PIN_SCK,
        .ws_io_num = I2S_PIN_WS,
        .data_out_num = I2S_PIN_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    ret = i2s_set_pin(s_i2s_port, &pin_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_set_pin failed: %d", ret);
        i2s_driver_uninstall(s_i2s_port);
        return ret;
    }

    ret = i2s_set_clk(s_i2s_port, g_sample_rate, I2S_BITS_PER_SAMPLE_32BIT, I2S_CHANNEL_STEREO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_set_clk failed: %d", ret);
        i2s_driver_uninstall(s_i2s_port);
        return ret;
    }

    ESP_LOGI(TAG, "audio_driver: initialized I2S port %d at %d Hz (32-bit, APLL)", s_i2s_port, g_sample_rate);

    // Prime I2S with silence
    i2s_zero_dma_buffer(s_i2s_port);
    static int32_t silence[256*2] = {0};
    size_t bytes_written;
    for (int k = 0; k < 6; k++) {
        i2s_write(s_i2s_port, (const char*)silence, sizeof(silence), &bytes_written, portMAX_DELAY);
    }

    // Initialize I2C and scan for ES8388
    esp_err_t i2c_ret = es8388_i2c_init();
    if (i2c_ret != ESP_OK) {
        ESP_LOGW(TAG, "ES8388 I2C init failed: %d (check SDA/SCL wiring)", i2c_ret);
        return i2c_ret;
    }

    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Scan for codec address
    uint8_t found_addr = scan_es8388();
    if (found_addr == 0) {
        ESP_LOGE(TAG, "ES8388 codec not found on I2C bus");
        return ESP_FAIL;
    }
    s_es8388_addr = found_addr;

    // Configure codec
    esp_err_t cfg_ret = es8388_configure_codec();
    if (cfg_ret != ESP_OK) {
        ESP_LOGE(TAG, "ES8388 configure failed: %d", cfg_ret);
        return cfg_ret;
    }

    ESP_LOGI(TAG, "ES8388 codec initialized successfully");
    return ESP_OK;
}

size_t audio_driver_write(const int32_t *samples, size_t sample_count) {
    if (samples == NULL || sample_count == 0) {
        return 0;
    }

    // The I2S peripheral is configured for stereo. Duplicate mono samples
    // into an interleaved stereo buffer (L,R,L,R,...) using 32-bit samples.
    size_t out_samples = sample_count * 2; // stereo
    size_t out_bytes = out_samples * sizeof(int32_t);

    int32_t *buf = (int32_t *)malloc(out_bytes);
    if (!buf) {
        ESP_LOGE(TAG, "audio_driver_write: malloc failed for %u bytes", (unsigned)out_bytes);
        return 0;
    }

    for (size_t i = 0; i < sample_count; ++i) {
        int32_t s = samples[i];
        buf[i * 2 + 0] = s; // left
        buf[i * 2 + 1] = s; // right
    }

    size_t bytes_written = 0;
    esp_err_t ret = i2s_write(s_i2s_port, buf, out_bytes, &bytes_written, portMAX_DELAY);
    free(buf);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_write failed: %d", ret);
        return 0;
    }

    // Keep analog path latched (some boards idle-mute) - every ~1 second
    static uint32_t last_keepalive = 0;
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
    if (now - last_keepalive > 1000) {
        last_keepalive = now;
        es8388_write_reg(0x04, 0x3C);  // HP + LOUT ON
        es8388_write_reg(0x19, 0x32);  // unmute + soft ramp
    }

    // bytes_written is number of bytes consumed from buf. Convert back to
    // mono sample count consumed: each mono sample produced 8 bytes in buf (2 x 32-bit).
    size_t samples_consumed = (bytes_written / (sizeof(int32_t) * 2));
    return samples_consumed;
}

void audio_driver_deinit(void) {
    ESP_LOGI(TAG, "audio_driver: deinit");
    i2s_driver_uninstall(s_i2s_port);
    g_sample_rate = 0;
}
