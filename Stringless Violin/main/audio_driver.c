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

// Default I2S pin assignment - update these to match your board wiring.
// If you're using a board/codec that provides alternate pins, change here
// or pass configuration in an expanded API.
#ifndef I2S_PIN_SCK
#define I2S_PIN_SCK  (GPIO_NUM_5)
#endif
#ifndef I2S_PIN_WS
#define I2S_PIN_WS   (GPIO_NUM_19)
#endif
#ifndef I2S_PIN_DOUT
#define I2S_PIN_DOUT (GPIO_NUM_18) // user wired module DIN to ESP GPIO18
#endif

// Default I2C pins for ES8388 control (user wired SDA to GPIO7)
#ifndef ES8388_I2C_SDA
#define ES8388_I2C_SDA (GPIO_NUM_7)
#endif
#ifndef ES8388_I2C_SCL
#define ES8388_I2C_SCL (GPIO_NUM_8) // change if you wired SCL elsewhere
#endif

// I2C port and 7-bit device address (update if your codec uses different addr)
#ifndef ES8388_I2C_PORT
#define ES8388_I2C_PORT I2C_NUM_0
#endif
#ifndef ES8388_I2C_ADDR
#define ES8388_I2C_ADDR 0x10
#endif

static bool s_es8388_i2c_inited = false;

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

// Write a single register to the ES8388 over I2C. Device address is ES8388_I2C_ADDR.
static esp_err_t es8388_write_reg(uint8_t reg, uint8_t val)
{
    if (!s_es8388_i2c_inited) {
        esp_err_t r = es8388_i2c_init();
        if (r != ESP_OK) return r;
    }

    uint8_t data[2] = { reg, val };
    esp_err_t ret = i2c_master_write_to_device(ES8388_I2C_PORT, ES8388_I2C_ADDR, data, sizeof(data), pdMS_TO_TICKS(1000));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "es8388_write_reg: failed to write reg 0x%02x: %d", reg, ret);
    }
    return ret;
}

// Placeholder for codec register configuration sequence.
// The ES8388 requires a specific set of register writes to power up the DAC,
// select I2S format and sample rate, and unmute/output the DAC. Fill this in
// based on the ES8388 datasheet or a tested driver sequence.
static esp_err_t es8388_configure_codec(void)
{
    // Example (commented) sequence - DO NOT rely on these values. Replace with
    // the correct initialization sequence from the ES8388 datasheet or driver.
    // es8388_write_reg(0x00, 0x00); // reset/power config (example)

    ESP_LOGI(TAG, "es8388_configure_codec: codec register init placeholder (no-op)");
    return ESP_OK;
}

esp_err_t audio_driver_init(int sample_rate) {
    esp_err_t ret;
    g_sample_rate = sample_rate;

    // Configure I2S for master TX, 16-bit samples. We configure stereo output
    // and duplicate mono samples to both channels in audio_driver_write().
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_TX,
        .sample_rate = g_sample_rate,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = 0,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    ret = i2s_driver_install(s_i2s_port, &i2s_config, 0, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "i2s_driver_install failed: %d", ret);
        return ret;
    }

    i2s_pin_config_t pin_config = {
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

    ESP_LOGI(TAG, "audio_driver: initialized I2S port %d at %d Hz", s_i2s_port, g_sample_rate);

    // NOTE: ES8388 (codec) also requires I2C register configuration to route
    // and enable DAC outputs. This code only configures I2S peripheral. Add
    // codec init here (or in another module) using the ES8388 driver/component
    // (typically via I2C) before sending audio samples.

    // Try to initialize I2C for ES8388 control using defaults (SDA=GPIO7).
    // This only sets up I2C; the codec register programming is a separate
    // sequence implemented in es8388_configure_codec().
    esp_err_t i2c_ret = es8388_i2c_init();
    if (i2c_ret == ESP_OK) {
        esp_err_t cfg_ret = es8388_configure_codec();
        if (cfg_ret != ESP_OK) {
            ESP_LOGW(TAG, "ES8388 configure sequence returned %d (codec may be uninitialized)", cfg_ret);
        } else {
            ESP_LOGI(TAG, "ES8388 codec configure sequence completed (placeholder)");
        }
    } else {
        ESP_LOGW(TAG, "ES8388 I2C init failed: %d (check SDA/SCL wiring)", i2c_ret);
    }

    return ESP_OK;
}

size_t audio_driver_write(const int16_t *samples, size_t sample_count) {
    if (samples == NULL || sample_count == 0) {
        return 0;
    }

    // The I2S peripheral is configured for stereo. Duplicate mono samples
    // into an interleaved stereo buffer (L,R,L,R,...). This allocates a
    // temporary buffer which is freed before returning. For higher
    // performance, consider a ring buffer or reusing a static buffer.
    size_t out_samples = sample_count * 2; // stereo
    size_t out_bytes = out_samples * sizeof(int16_t);

    int16_t *buf = (int16_t *)malloc(out_bytes);
    if (!buf) {
        ESP_LOGE(TAG, "audio_driver_write: malloc failed for %u bytes", (unsigned)out_bytes);
        return 0;
    }

    for (size_t i = 0; i < sample_count; ++i) {
        int16_t s = samples[i];
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

    // bytes_written is number of bytes consumed from buf. Convert back to
    // mono sample count consumed: each mono sample produced 4 bytes in buf.
    size_t samples_consumed = (bytes_written / (sizeof(int16_t) * 2));
    return samples_consumed;
}

void audio_driver_deinit(void) {
    ESP_LOGI(TAG, "audio_driver: deinit");
    i2s_driver_uninstall(s_i2s_port);
    g_sample_rate = 0;
}
