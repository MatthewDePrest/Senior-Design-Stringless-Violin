#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include "main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "audio_driver.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ---------- Audio Configuration ----------
static const uint32_t SAMPLE_RATE = 48000;
static const size_t FRAMES = 100; // used to be 256
static int VOLUME_Q15 = 1600;

// Violin-ish harmonic profile
static const float HARM[] = {1.00f, 0.38f, 0.20f, 0.12f, 0.08f, 0.05f};
#define NH 6

// Vibrato settings
static bool g_vibrato_on = true;
static float g_vib_rate_hz = 5.2f;
static float g_vib_depth_cents = 12.0f;
static float volMod = 3.0f;
// Low-pass filter
static bool g_lpf_on = true;
static float g_lpf_cut_hz = 4500.0f;

// ---------- 1-pole Low-Pass Filter ----------
typedef struct {
    float a;
    float y;
} OnePoleLPF;

static void lpf_set_cutoff(OnePoleLPF* lpf, float fc_hz) {
    float a_ = expf(-2.0f * (float)M_PI * fc_hz / (float)SAMPLE_RATE);
    lpf->a = fminf(fmaxf(a_, 0.0f), 0.9999f);
}

static inline float lpf_process(OnePoleLPF* lpf, float x) {
    lpf->y = (1.0f - lpf->a) * x + lpf->a * lpf->y;
    return lpf->y;
}

// ---------- Reverb (Schroeder: 4 combs + 2 allpasses) ----------
typedef struct {
    float* buf;
    int len;
    int idx;
    float fb;
    float damp;
    float fil;
} Comb;

static inline float comb_process(Comb* c, float x) {
    float y = c->buf[c->idx];
    c->fil = (1.0f - c->damp) * y + c->damp * c->fil;
    c->buf[c->idx] = x + c->fil * c->fb;
    c->idx++;
    if (c->idx >= c->len) c->idx = 0;
    return y;
}

typedef struct {
    float* buf;
    int len;
    int idx;
    float g;
} Allpass;

static inline float allpass_process(Allpass* a, float x) {
    float y = a->buf[a->idx];
    float out = -x + y;
    a->buf[a->idx] = x + y * a->g;
    a->idx++;
    if (a->idx >= a->len) a->idx = 0;
    return out;
}

typedef struct {
    float c1[1439];
    float c2[1601];
    float c3[1867];
    float c4[2053];
    float a1[225];
    float a2[341];
    
    Comb combs[4];
    Allpass aps[2];
    
    bool enabled;
    float mix;
    float room;
    float damp;
} ReverbSC;

static void reverb_init(ReverbSC* rev) {
    memset(rev->c1, 0, sizeof(rev->c1));
    memset(rev->c2, 0, sizeof(rev->c2));
    memset(rev->c3, 0, sizeof(rev->c3));
    memset(rev->c4, 0, sizeof(rev->c4));
    memset(rev->a1, 0, sizeof(rev->a1));
    memset(rev->a2, 0, sizeof(rev->a2));
    
    rev->combs[0] = (Comb){rev->c1, 1439, 0, rev->room, rev->damp, 0.0f};
    rev->combs[1] = (Comb){rev->c2, 1601, 0, rev->room, rev->damp, 0.0f};
    rev->combs[2] = (Comb){rev->c3, 1867, 0, rev->room, rev->damp, 0.0f};
    rev->combs[3] = (Comb){rev->c4, 2053, 0, rev->room, rev->damp, 0.0f};
    
    rev->aps[0] = (Allpass){rev->a1, 225, 0, 0.5f};
    rev->aps[1] = (Allpass){rev->a2, 341, 0, 0.5f};
}

static inline float reverb_process(ReverbSC* rev, float x) {
    if (!rev->enabled) return x;
    
    float s = 0.0f;
    s += comb_process(&rev->combs[0], x);
    s += comb_process(&rev->combs[1], x);
    s += comb_process(&rev->combs[2], x);
    s += comb_process(&rev->combs[3], x);
    s *= 0.25f;
    
    s = allpass_process(&rev->aps[0], s);
    s = allpass_process(&rev->aps[1], s);
    
    return (1.0f - rev->mix) * x + rev->mix * s;
}

// ---------- Per-String Note State ----------
typedef struct {
    float f0;
    float vib_phase;
    float vib_inc;
    float phase[NH];
} StringState;

// ---------- Helper Functions ----------
static inline float cents_to_ratio(float cents) {
    return powf(2.0f, cents / 1200.0f);
}

// SINGLE fill_violin_buffer - takes bow speed from remote IMU
static void fill_violin_buffer(int32_t* buf, size_t frames, allData* data, 
                                StringState* strings, OnePoleLPF* lpf, ReverbSC* rev) {
    const float twoPi = 2.0f * (float)M_PI;
    
    // Get bow speed from remote IMU (ESP-XIAO) with NULL check
    float bowSpeed = 0.5f;  // Default fallback
    ImuPacket *remote = get_remote_imu();
    if (remote != NULL) {
        float remote_accel_mag = sqrtf(remote->imu.ax * remote->imu.ax + 
                                    remote->imu.ay * remote->imu.ay + 
                                    remote->imu.az * remote->imu.az);
        bowSpeed = fminf(remote_accel_mag / 20.0f, 1.0f);
    }
    
    for (size_t i = 0; i < frames; ++i) {
        float sample = 0.0f;
        
        for (int s = 0; s < 4; s++) {
            if(s!=0){
                continue;
            }
            if (data->pressures[s] <= 10) continue;
            
            float f0 = strings[s].f0;
            
            float f = f0;
            if (g_vibrato_on) {
                float cents = g_vib_depth_cents * sinf(strings[s].vib_phase);
                strings[s].vib_phase += strings[s].vib_inc;
                if (strings[s].vib_phase > twoPi) strings[s].vib_phase -= twoPi;
                f *= cents_to_ratio(cents);
            }
            
            float string_sample = 0.0f;
            for (int h = 0; h < NH; ++h) {
                float fh = f * (float)(h + 1);
                if (fh > 0.45f * (float)SAMPLE_RATE) break;
                float inc = twoPi * fh / (float)SAMPLE_RATE;
                strings[s].phase[h] += inc;
                if (strings[s].phase[h] > twoPi) strings[s].phase[h] -= twoPi;
                string_sample += HARM[h] * sinf(strings[s].phase[h]);
            }
            
            float amplitude = volMod * bowSpeed * ((float)data->pressures[s] / 1023.0f);
            sample += string_sample * amplitude;
        }
        
        if (g_lpf_on) {
            sample = lpf_process(lpf, sample);
        }
        
        sample = reverb_process(rev, sample);
        
        int16_t s16 = (int16_t) fmaxf(fminf(sample * (float)VOLUME_Q15, 32767.0f), -32768.0f);
        int32_t s32 = ((int32_t)s16) << 16;
        
        buf[i] = s32;
    }
}

const char* frequencyToNote(float freq) {
    const float A4 = 440.0f;
    float semitonesFromA4 = 12.0f * log2f(freq / A4);
    int midi = (int)roundf(69.0f + semitonesFromA4);
    const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    
    int noteIndex = midi % 12;
    int octave = midi / 12 - 1;

    static char note[16];
    snprintf(note, sizeof(note), "%s%d", noteNames[noteIndex], octave);
    return note;
}

const char* getNoteFromPressureAndFreq(float* freqs, int* pressures) {
    int highestPressureIndex = 0;
    for (int i = 1; i < 4; i++) {
        if (pressures[i] >= pressures[highestPressureIndex]) {
            highestPressureIndex = i;
        }
    }
    float freq = freqs[highestPressureIndex];
    return frequencyToNote(freq);
}

void appendNoteToFile(const char* note) {
    printf("Note: %s\n", note);
}

void output(void *pvParameters) {
    allData *data = (allData *)pvParameters;

    const int buf_samples = (int)FRAMES;
    audio_driver_init(SAMPLE_RATE);

    int32_t *pcm = malloc(sizeof(int32_t) * buf_samples);
    if (!pcm) {
        printf("output: failed to allocate pcm buffer\n");
        vTaskDelete(NULL);
        return;
    }

    StringState strings[4];
    memset(strings, 0, sizeof(strings));
    
    OnePoleLPF lpf = {0};
    lpf_set_cutoff(&lpf, g_lpf_cut_hz);
    
    ReverbSC *rev = malloc(sizeof(ReverbSC));
    if (!rev) {
        printf("output: failed to allocate reverb buffer\n");
        free(pcm);
        vTaskDelete(NULL);
        return;
    }
    rev->enabled = false;
    rev->mix = 0.25f;
    rev->room = 0.82f;
    rev->damp = 0.30f;
    reverb_init(rev);

    for (int s = 0; s < 4; s++) {
        strings[s].vib_inc = (2.0f * (float)M_PI * g_vib_rate_hz) / (float)SAMPLE_RATE;
    }

    uint32_t debug_frames = 0;
    printf("output: Audio synthesis task started. Sample rate=%lu Hz block=%d\n", (unsigned long)SAMPLE_RATE, buf_samples);

    while (!data->end) {
        noteConversion(data);
        
        for (int s = 0; s < 4; s++) {
            strings[s].f0 = data->stringsFreqs[s];
        }

        debug_frames++;
        
        if (debug_frames % 10000 == 0) {
            printf("pos: [%.1f, %.1f, %.1f, %.1f]  freq: [%.1f, %.1f, %.1f, %.1f]\n",
                data->positions[0], data->positions[1], data->positions[2], data->positions[3],
                data->stringsFreqs[0], data->stringsFreqs[1], data->stringsFreqs[2], data->stringsFreqs[3]);
            printf("press: [%d, %d, %d, %d]\n",
                data->pressures[0], data->pressures[1], data->pressures[2], data->pressures[3]);
        }
        if (debug_frames % 100 == 0) {        
            const char* note = getNoteFromPressureAndFreq(data->stringsFreqs, data->pressures);
            appendNoteToFile(note);
        }

        fill_violin_buffer(pcm, buf_samples, data, strings, &lpf, rev);
        audio_driver_write(pcm, buf_samples);
    }

    free(pcm);
    free(rev);
    audio_driver_deinit();
    vTaskDelete(NULL);
}