# ARCHITECTURE.md — SPECTER System Architecture

---

## 1. System Overview

SPECTER is a distributed embedded system with three compute nodes connected via serial buses.

```
┌─────────────────────────────────────────────────────────────────────┐
│                        HARDWARE LAYER                               │
│                                                                     │
│  Voltage Sensor (25V divider) ─ADC─┐                              │
│  (resistive divider → Teensy A0)    ├──► Teensy 4.1               │
│  Current Sensor (INA219) ───I2C────┘    (600 MHz Cortex-M7)       │
│                                            │                        │
│                                        USB Serial                   │
│                                        (10B frames, 3.2 kHz)        │
│                                            │                        │
│                                            ▼                        │
│                                    Raspberry Pi 5                   │
│                                    (4× Cortex-A76, 8 GB)           │
│                                            │                        │
│                                         UART                        │
│                                        (JSON, 10 Hz)               │
│                                            │                        │
│                                            ▼                        │
│                                       ESP32-S3                      │
│                                    (WiFi AP + WebSocket)            │
│                                            │                        │
│                                         WiFi                        │
│                                            ▼                        │
│                                    Judge's Phone                    │
│                                    (Browser dashboard)              │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 2. Raspberry Pi Process Architecture

Four worker processes mapped to four CPU cores, plus the orchestrator.

```
┌────────────────────────────────────────────────────────────────────┐
│  Raspberry Pi 5                                                    │
│                                                                    │
│  Core 0: [Acquisition Process]          Core 2: [Tracking Process]│
│  ├── acquire.py                         ├── kalman.py              │
│  │   USB serial reader                  │   80-dim EKF at 100 Hz  │
│  │   Frame validation                   │   CUSUM accumulator      │
│  │   Ring buffer writer                 ├── crossmodal.py          │
│  └── (asyncio, I/O bound)               │   Ohm's Law consistency  │
│                                         └── Coeff writer → shm    │
│  Core 1: [Compensation Process]                                    │
│  ├── compensate.py                      Core 3: [Services Process] │
│  │   Volterra filter hot path           ├── bootstrap.py           │
│  │   Wiener FIR hot path               │   Bispectrum estimation  │
│  │   Calibrated output writer           │   Volterra kernel solve  │
│  └── (fully vectorised NumPy)          │   Wiener tap compute     │
│                                         ├── forensics.py           │
│  ORCHESTRATOR (main.py):               │   Forensics report gen   │
│  ├── Spawns+monitors all workers       ├── telemetry.py            │
│  ├── State machine controller          │   UART→ESP32 at 10 Hz    │
│  └── Routes CUSUM→re-bootstrap        └── (on-demand, CPU heavy) │
│                                                                    │
│  ┌────────────────── SHARED MEMORY REGIONS ───────────────────┐   │
│  │  shm_ring_buffer:    [timestamp|voltage|current] × 4096    │   │
│  │  shm_coefficients:   [w_wiener:64×f64 | h2:16×f64]       │   │
│  │  shm_output:         [v_raw|v_corr|i_raw|i_corr] × 256    │   │
│  │  shm_telemetry:      current telemetry JSON frame          │   │
│  │  shm_heartbeat:      [acq|comp|track|svc] × uint64        │   │
│  └────────────────────────────────────────────────────────────┘   │
└────────────────────────────────────────────────────────────────────┘
```

---

## 3. DSP Pipeline (Raspberry Pi — Algorithm Chain)

Called once per 512-sample frame. Runs on Core 1 as fast as possible.

```
Raw Samples (512 × int16 from ring buffer)
        │
        ▼
  [Hanning Window × 512]
        │
        ▼
  [512-point FFT]  ──────────────────┐ (frames for bispectrum,
        │                            │  runs during BOOTSTRAP only)
        ▼                            ▼
  [Volterra Filter]          [Bispectrum Accumulation]
  y_v[n] = Σ h1[τ]·x[n-τ]  B̂(k1,k2) = X(k1)·X(k2)·X*(k1+k2)
          + ΣΣ h2[τ1,τ2]·x[n-τ1]·x[n-τ2]  averaged over M frames
        │
        ▼
  [Wiener FIR Equalizer]
  z[n] = w^T · x_volt_buffer
  (64-tap FIR, coefficients from EKF state)
        │
        ▼
  [Cross-Modal Error]
  ε = ||V_corrected - I_corrected × R_load||²
        │
        ▼
  Calibrated Output  ──► shared memory → telemetry → ESP32
        │
        ▼
  [EKF Measurement Update (Core 2, 100 Hz)]
  Uses ε as measurement, updates 80-dim state vector
  (64 Wiener taps + 16 Volterra quadratic coefficients)
```

---

## 4. Bootstrap Sequence (detailed)

Triggered: on COLD_BOOT, on CUSUM fire, on SENSOR_SWAP.

```
step 1: Collect 100 frames × 512 samples = 51,200 samples
step 2: Hanning window each frame
step 3: 512-pt FFT per frame (NumPy rfft)
step 4: Bispectrum B̂(k1,k2) = X(k1)·X(k2)·conj(X(k1+k2)) in principal domain
step 5: Average over 100 frames → B̄(k1,k2)
step 6: Estimate linear kernel H1(ω) from power spectrum
step 7: Extract quadratic kernel H2(ω1,ω2) from bispectrum
step 8: Discretize Fredholm equation → form matrix A, vector b
step 9: Tikhonov solve: (A^T·A + λ_reg·I)·h = A^T·b → get inverse kernels
step 10: Compute W(ω) = H1*(ω)/(|H1(ω)|² + λ) at 64 frequencies
step 11: w_taps = IFFT64(W(ω)) → 64-tap Wiener FIR coefficients
step 12: Initialize EKF state: x[0..63] = w_taps, x[64..79] = h2_dominant
step 13: Initialize EKF covariance: P = 1e-4 · I(80)
step 14: Atomic coefficient swap → system enters COMPENSATING state
step 15: Generate forensics report
Total target: < 8 seconds on Pi 5
```

---

## 5. State Machine

```
                     ┌──────────────────┐
            Power on │                  │
         ────────────►   COLD_BOOT      │
                     │                  │
                     │ Init comms       │
                     │ Fill ring buffer │
                     └────────┬─────────┘
                              │ buffer ≥ 512 samples
                              ▼
                     ┌──────────────────┐
                     │   BOOTSTRAP      │◄────────────────────┐
                     │                  │                     │
                     │ Bispectrum       │                     │
                     │ Volterra solve   │                     │
                     │ Wiener taps      │                     │
                     │ EKF init         │                     │
                     └────────┬─────────┘                     │
                              │ < 8 seconds                   │
                              ▼                               │
                     ┌──────────────────┐   CUSUM fires  ┌───┴──────────────┐
                     │  COMPENSATING    ├───────────────► │  RE_BOOTSTRAP   │
                     │                  │                 │                  │
                     │ Volterra+Wiener  │  new sensor     │ Background ID   │
                     │ EKF at 100 Hz   │ detected        │ Last-known-good │
                     │ CUSUM monitor   ├────────────┐    │ filter active   │
                     │ Cross-modal     │            │    └─────────────────┘
                     └──────────────────┘            │
                                                     ▼
                                            ┌────────────────┐
                                            │  SENSOR_SWAP   │
                                            │                │
                                            │ Full cold boot │
                                            │ on new sensor  │
                                            └────────────────┘
```

### State Transition Table

| From | Event | To | Action |
|---|---|---|---|
| COLD_BOOT | ring_buffer_ready | BOOTSTRAP | spawn bootstrap task |
| BOOTSTRAP | bootstrap_success | COMPENSATING | atomic coeff swap, start EKF |
| BOOTSTRAP | bootstrap_timeout (>30s) | COLD_BOOT | reset, try again |
| COMPENSATING | cusum_fired | RE_BOOTSTRAP | start background bootstrap, keep old coefficients |
| COMPENSATING | sensor_disconnect | COLD_BOOT | reset ring buffer |
| COMPENSATING | sensor_signature_change | SENSOR_SWAP | force cold bootstrap |
| RE_BOOTSTRAP | new_bootstrap_ready | COMPENSATING | atomic coeff swap |
| SENSOR_SWAP | bootstrap_success | COMPENSATING | atomic coeff swap |

---

## 6. Teensy Firmware Architecture

```
main.cpp
├── setup()
│   ├── Serial.begin()           // USB CDC at 12 Mbps
│   ├── analogReadResolution(12) // voltage divider on ADC pin A0 (no SPI needed)
│   ├── Wire.begin()             // I2C for INA219
│   ├── INA219_configure()       // Set conversion mode, gain, averaging
│   ├── IntervalTimer.begin(sample_ISR, SAMPLE_PERIOD_US)
│   └── Serial.println("READY")
│
└── loop()
    ├── if (g_buffer_ready) transmit_buffer_to_pi()
    └── (nothing else — timing is ISR-driven)

sample_ISR()   [called at SAMPLE_RATE_HZ = 3200 Hz from IntervalTimer]
├── voltage_raw = analogRead(VOLTAGE_PIN) → 12-bit value
├── current_raw = g_latest_current_raw  // updated outside ISR by non-blocking I2C poll
├── timestamp_us = micros()
├── write raw sample triplet to active ping-pong buffer
├── if buffer full: set g_buffer_ready = true, swap buffers
└── (ISR returns, < 1 μs total; no I2C, no serial, no dynamic allocation)

transmit_buffer_to_pi()  [called from loop()]
├── for each sample in inactive buffer:
│   ├── compute XOR checksum
│   └── write 10-byte frame to USB serial
└── clear g_buffer_ready

poll_ina219_nonblocking()  [called from loop()]
├── check conversion-ready bit
├── if ready: read current register over I2C
└── update g_latest_current_raw (volatile)
```

---

## 7. ESP32-S3 Firmware Architecture

```
main.cpp
├── setup()
│   ├── WiFi.softAP("SPECTER-DEMO", "", 1, false, 8)
│   ├── WebSocket server on port 80
│   ├── serve dashboard HTML from SPIFFS flash
│   ├── Serial2.begin(115200)    // UART from Pi
│   └── OLED and LED strip init
│
└── loop()
    ├── if Serial2.available(): read_telemetry_from_pi()
    ├── if telemetry_updated: broadcast_to_websocket_clients()
    └── update_oled_and_leds()

read_telemetry_from_pi()
├── read until '\n' delimiter
├── parse JSON telemetry frame
└── store in global TelemetryState struct

broadcast_to_websocket_clients()
└── ws.textAll(telemetry_json_string)
```

---

## 8. Inter-Process Communication Summary

| Channel | Type | Direction | Rate | Data |
|---|---|---|---|---|
| USB Serial | `pyserial` + framing | Teensy → Pi Acq | 3.2 kHz | **10-byte** sample frames (see DATA_MODELS.md §1) |
| ACQ→COMP | `shared_memory` ring buffer | Acq → Comp | continuous | raw sample pairs + timestamps |
| COMP→TRACK | `shared_memory` output | Comp → Track | 100 Hz | corrected V, I, cross-modal ε |
| TRACK→COMP | `shared_memory` coefficients + `multiprocessing.Lock` | Track → Comp | 100 Hz | 80-dim coeff vector |
| SERVICES←TRACK | `multiprocessing.Queue` | Track → Services | on event | CUSUM fire events |
| SERVICES←MAIN | `multiprocessing.Queue` | Main → Services | on event | bootstrap trigger commands |
| UART | `pyserial` UART | Pi → ESP32 | 10 Hz | JSON telemetry frames |
| WebSocket | `ESPAsyncWebServer` | ESP32 → Browsers | 10 Hz | same JSON telemetry |

---

## 9. Shared Memory Layout

```python
# specter/shared_memory_layout.py — canonical layout

RING_BUFFER_CAPACITY = 4096   # samples
COEFF_DIM = 80                # 64 Wiener taps + 16 Volterra coefficients
OUTPUT_BUFFER_CAPACITY = 256  # calibrated output samples

# shm_ring: ring buffer of SampleRecord
# SampleRecord: [timestamp_us: uint32, voltage: int16, current: int16] = 8 bytes
SHM_RING_SIZE_BYTES = RING_BUFFER_CAPACITY * 8

# shm_coefficients: [coefficients: float64×80, lock_flag: uint8]
SHM_COEFF_SIZE_BYTES = COEFF_DIM * 8 + 1

# shm_output: [v_raw: float32, v_corr: float32, i_raw: float32, i_corr: float32] × N
SHM_OUTPUT_SIZE_BYTES = OUTPUT_BUFFER_CAPACITY * 16

# shm_telemetry: JSON string buffer, max 2048 bytes
SHM_TELEMETRY_SIZE_BYTES = 2048

# shm_heartbeat: [acq_ts, comp_ts, track_ts, svc_ts] × uint64 (Unix epoch ms)
SHM_HEARTBEAT_SIZE_BYTES = 4 * 8
```

---

## 10. Key Design Decisions and Rationale

| Decision | Rationale |
|---|---|
| Bispectrum for blind ID | Only HOS method that extracts nonlinear sensor fingerprint without any reference signal |
| Volterra 2nd-order truncation | Captures >95% of nonlinear energy in MEMS sensors at manageable compute cost |
| Fredholm + Tikhonov | Closed-form, non-iterative inverse. Tikhonov prevents ill-conditioned solve explosion |
| Wiener deconvolution (not CMA) | CMA requires constant-modulus signal (not true for physical sensors). Wiener has no local minima |
| EKF coefficients embedded in state | Drift tracking and equalizer adaptation are one mechanism, not two |
| Python multiprocessing (not asyncio for compute) | GIL limitation: asyncio shares GIL across all tasks. Multiprocessing bypasses GIL for true parallelism |
| Shared memory (not Queue) for samples | Queue serializes via pickle — too slow for 3.2 kHz sample data |
| Teensy (not Pi GPIO) for acquisition | Pi runs Linux — no real-time guarantee. Teensy: deterministic < 100 ns jitter |
| Cross-modal via Ohm's Law | Exact physics constraint, zero cost, couples two independent calibrations |
