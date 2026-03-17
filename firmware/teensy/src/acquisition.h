#pragma once

#include <Arduino.h>
#include <IntervalTimer.h>
#include <stdint.h>

constexpr uint32_t SAMPLE_RATE_HZ = 3200;
constexpr uint32_t SAMPLE_PERIOD_US = 1000000 / SAMPLE_RATE_HZ;
constexpr uint16_t BUFFER_SIZE_SAMPLES = 512;
constexpr uint8_t VOLTAGE_ADC_PIN = A0;
constexpr uint8_t SAMPLE_RATE_DIAG_PIN = 3;  // PWM-capable header pin reserved for 3.2 kHz oscilloscope validation.
constexpr uint8_t SYNC_BYTE = 0xAA;

#pragma pack(push, 1)
struct SampleFrame {
    uint8_t sync;
    uint32_t timestamp_us;
    int16_t voltage_raw;
    int16_t current_raw;
    uint8_t checksum;
};
#pragma pack(pop)

static_assert(sizeof(SampleFrame) == 10, "SampleFrame must be exactly 10 bytes");

extern volatile bool g_buffer_ready;
extern volatile uint8_t g_active_buffer_idx;
extern volatile int16_t g_latest_current_raw;
extern int16_t g_voltage_buf[2][BUFFER_SIZE_SAMPLES];
extern int16_t g_current_buf[2][BUFFER_SIZE_SAMPLES];
extern uint32_t g_timestamp_buf[2][BUFFER_SIZE_SAMPLES];

void ISR_sample_tick();
void acquisition_init();
