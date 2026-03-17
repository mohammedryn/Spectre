/**
 * @file main.cpp
 * @brief SPECTER C-03-E — Acquisition timer frequency and jitter measurement.
 *
 * Replaces an oscilloscope. Captures 1025 rising edges of the Teensy 4.1
 * diagnostic PWM signal on SAMPLE_RATE_DIAG_PIN (Pin 3), which is driven as a
 * hardware FlexPWM at exactly SAMPLE_RATE_HZ (3200 Hz) by acquisition_init().
 *
 * Technique: timestamp every rising edge with the Xtensa LX7 CPU cycle counter
 * (CCOUNT register, ~4.17 ns resolution at 240 MHz), then compute:
 *   - measured frequency   → must be 3200.00 ± 0.1 Hz
 *   - jitter RMS           → must be < 100 ns
 *
 * Hardware connections:
 *   Teensy Pin 3  ──[100Ω series resistor]── ESP32 SIGNAL_GPIO_PIN
 *   Teensy GND    ─────────────────────────── ESP32 GND
 *   (Both boards run at 3.3 V logic — no level shifter needed.)
 *
 * How to find a usable GPIO on the ESP32-S3-BOX-3:
 *   The PMOD expansion connector (30-pin, bottom of board) exposes
 *   GPIO 40–47 on revision C. Check your board's silkscreen or schematic at:
 *   https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/
 *   Change SIGNAL_GPIO_PIN below to whichever pin you can reach with a wire.
 */

#include <Arduino.h>

// ── User configuration ──────────────────────────────────────────────────────
// Set this to the GPIO number you physically connected Teensy Pin 3 to.
static constexpr int SIGNAL_GPIO_PIN = 40;

// Measurement parameters (do not change unless brief says otherwise)
static constexpr int    N_CAPTURE        = 1025;   // edges → 1024 period samples
static constexpr double TARGET_FREQ_HZ   = 3200.0;
static constexpr double FREQ_TOL_HZ      =    0.1;
static constexpr double JITTER_LIMIT_NS  =  100.0;
// ────────────────────────────────────────────────────────────────────────────

static volatile uint32_t s_ts[N_CAPTURE];  // captured CCOUNT timestamps
static volatile uint32_t s_idx  = 0;
static volatile bool     s_done = false;

/**
 * Read the Xtensa LX7 CPU cycle counter (CCOUNT).
 * At 240 MHz this ticks every ~4.17 ns — sufficient for < 100 ns jitter.
 * Placed in IRAM so interrupt latency stays constant even during flash accesses.
 */
static inline uint32_t IRAM_ATTR ccount() {
    uint32_t v;
    asm volatile("rsr.ccount %0" : "=a"(v));
    return v;
}

/**
 * Rising-edge ISR: capture a CPU timestamp for each edge, then stop.
 * Kept minimal — one register read, one store, one compare — to minimise
 * the ISR-induced jitter contribution to our own measurement.
 */
void IRAM_ATTR isr_rising_edge() {
    if (!s_done) {
        s_ts[s_idx] = ccount();
        if (++s_idx >= static_cast<uint32_t>(N_CAPTURE)) {
            s_done = true;
        }
    }
}

/** Compute statistics from captured timestamps and print pass/fail report. */
static void run_analysis() {
    const uint32_t cpu_hz = getCpuFrequencyMhz() * 1000000UL;
    const int N = N_CAPTURE - 1;  // 1024 period measurements

    // ── Pass 1: mean period in cycles ────────────────────────────────────────
    double sum_cyc = 0.0;
    for (int i = 0; i < N; i++) {
        // uint32_t wraps gracefully; subtraction still gives the correct delta.
        sum_cyc += static_cast<double>(s_ts[i + 1] - s_ts[i]);
    }
    const double mean_cyc = sum_cyc / N;
    const double mean_ns  = mean_cyc * 1e9 / cpu_hz;
    const double meas_hz  = static_cast<double>(cpu_hz) / mean_cyc;

    // ── Pass 2: variance, min, max ───────────────────────────────────────────
    double min_cyc = static_cast<double>(s_ts[1] - s_ts[0]);
    double max_cyc = min_cyc;
    double sum_sq  = 0.0;
    for (int i = 0; i < N; i++) {
        const double d   = static_cast<double>(s_ts[i + 1] - s_ts[i]);
        const double dev = d - mean_cyc;
        sum_sq += dev * dev;
        if (d < min_cyc) min_cyc = d;
        if (d > max_cyc) max_cyc = d;
    }
    const double jitter_rms_ns  = sqrt(sum_sq / N) * 1e9 / cpu_hz;
    const double jitter_pkpk_ns = (max_cyc - min_cyc) * 1e9 / cpu_hz;

    // ── Report ───────────────────────────────────────────────────────────────
    Serial.println(F("\n+── C-03-E JITTER MEASUREMENT RESULTS ───────────────────────+"));
    Serial.printf( "| GPIO under test  : %d  (Teensy Pin 3 / SAMPLE_RATE_DIAG_PIN)\n", SIGNAL_GPIO_PIN);
    Serial.printf( "| CPU frequency    : %u MHz\n",  getCpuFrequencyMhz());
    Serial.printf( "| Periods measured : %d\n", N);
    Serial.println(F("|"));
    Serial.printf( "| Mean period      : %12.2f ns\n", mean_ns);
    Serial.printf( "| Expected period  : %12.2f ns   (1 / 3200 Hz)\n", 1e9 / TARGET_FREQ_HZ);
    Serial.printf( "| Measured freq    : %12.4f Hz\n", meas_hz);
    Serial.printf( "| Freq error       : %+12.4f Hz   (limit +/-%.1f Hz)\n",
                   meas_hz - TARGET_FREQ_HZ, FREQ_TOL_HZ);
    Serial.println(F("|"));
    Serial.printf( "| Jitter RMS       : %12.2f ns   (limit %.0f ns)\n",
                   jitter_rms_ns, JITTER_LIMIT_NS);
    Serial.printf( "| Jitter pk-pk     : %12.2f ns\n",   jitter_pkpk_ns);
    Serial.printf( "| Min period       : %12.1f ns\n",
                   min_cyc * 1e9 / cpu_hz);
    Serial.printf( "| Max period       : %12.1f ns\n",
                   max_cyc * 1e9 / cpu_hz);
    Serial.println(F("|"));

    const bool freq_pass   = fabs(meas_hz - TARGET_FREQ_HZ) <= FREQ_TOL_HZ;
    const bool jitter_pass = jitter_rms_ns <= JITTER_LIMIT_NS;

    Serial.printf( "| Frequency  : %s   (%.4f Hz, limit +/-%.1f Hz)\n",
                   freq_pass   ? "PASS" : "FAIL", meas_hz, FREQ_TOL_HZ);
    Serial.printf( "| Jitter RMS : %s   (%.2f ns, limit %.0f ns)\n",
                   jitter_pass ? "PASS" : "FAIL", jitter_rms_ns, JITTER_LIMIT_NS);
    Serial.println(F("|"));
    if (freq_pass && jitter_pass) {
        Serial.println(F("|   >>> C-03-E: ALL PASS  --  Task validated. <<<"));
    } else {
        Serial.println(F("|   >>> C-03-E: FAIL  --  See notes in guide.  <<<"));
    }
    Serial.println(F("+─────────────────────────────────────────────────────────────+"));
}

void setup() {
    // Fix CPU at 240 MHz so that ccount resolution stays constant.
    setCpuFrequencyMhz(240);

    Serial.begin(115200);
    delay(2000);  // Give USB-CDC time to enumerate on host

    Serial.println(F("\n=== SPECTER C-03-E: Acquisition Timer Jitter Measurement ==="));
    Serial.printf("Waiting for %d rising edges on GPIO %d...\n",
                  N_CAPTURE, SIGNAL_GPIO_PIN);
    Serial.printf("Expected capture time: ~%.0f ms\n\n",
                  static_cast<double>(N_CAPTURE) / TARGET_FREQ_HZ * 1000.0);

    pinMode(SIGNAL_GPIO_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(SIGNAL_GPIO_PIN),
                    isr_rising_edge, RISING);
}

void loop() {
    if (s_done) {
        detachInterrupt(digitalPinToInterrupt(SIGNAL_GPIO_PIN));
        run_analysis();
        for (;;) delay(5000);  // Halt — results stay on serial monitor
    }
}
