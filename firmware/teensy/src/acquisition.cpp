#include "acquisition.h"

volatile bool g_buffer_ready = false;
volatile uint8_t g_active_buffer_idx = 0;
volatile int16_t g_latest_current_raw = 0;
int16_t g_voltage_buf[2][BUFFER_SIZE_SAMPLES] = {};
int16_t g_current_buf[2][BUFFER_SIZE_SAMPLES] = {};
uint32_t g_timestamp_buf[2][BUFFER_SIZE_SAMPLES] = {};

namespace {

IntervalTimer g_sample_timer;
volatile uint16_t g_buffer_write_pos = 0;

}  // namespace

void ISR_sample_tick() {
    const uint8_t buffer_index = g_active_buffer_idx;
    const uint16_t write_pos = g_buffer_write_pos;

    g_voltage_buf[buffer_index][write_pos] = static_cast<int16_t>(analogRead(VOLTAGE_ADC_PIN));
    g_current_buf[buffer_index][write_pos] = g_latest_current_raw;
    g_timestamp_buf[buffer_index][write_pos] = micros();

    const uint16_t next_write_pos = static_cast<uint16_t>(write_pos + 1U);
    if (next_write_pos >= BUFFER_SIZE_SAMPLES) {
        g_buffer_write_pos = 0;
        g_active_buffer_idx = static_cast<uint8_t>(1U - buffer_index);
        g_buffer_ready = true;
        return;
    }

    g_buffer_write_pos = next_write_pos;
}

void acquisition_init() {
    g_buffer_ready = false;
    g_active_buffer_idx = 0;
    g_latest_current_raw = 0;
    g_buffer_write_pos = 0;

    analogReadResolution(12);
    analogReadAveraging(1);
    pinMode(SAMPLE_RATE_DIAG_PIN, OUTPUT);
    analogWriteFrequency(SAMPLE_RATE_DIAG_PIN, SAMPLE_RATE_HZ);
    analogWrite(SAMPLE_RATE_DIAG_PIN, 128);
    g_sample_timer.begin(ISR_sample_tick, SAMPLE_PERIOD_US);
}
