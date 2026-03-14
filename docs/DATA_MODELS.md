# DATA_MODELS.md — SPECTER

All data structures, packet formats, enums, and shared memory layouts. Canonical definitions.

---

## 1. Teensy USB Serial Packet

**10 bytes per packet, little-endian**

> Note: The PRD (section 8.2) states "9 bytes" because it omits the sync byte. SPECTER adds a sync byte (0xAA) as byte 0 for robust frame re-synchronization after serial dropout. Canonical size is **10 bytes**.

| Byte(s) | Field | Type | Description |
|---|---|---|---|
| 0 | sync | `uint8_t` | Always `0xAA` — marks packet start |
| 1–4 | timestamp_us | `uint32_t` | Microsecond timestamp from `micros()`, wraps ~71 min |
| 5–6 | voltage_raw | `int16_t` | ADC reading: 0–4095 for 12-bit, scaled to int16 |
| 7–8 | current_raw | `int16_t` | INA219 signed current in raw units (1 LSB = 10 μA) |
| 9 | checksum | `uint8_t` | XOR of bytes 0–8 (all preceding bytes, sync included) |

**Teensy C++ struct:**
```cpp
#pragma pack(push, 1)
struct SampleFrame {
    uint8_t  sync;          // 0xAA
    uint32_t timestamp_us;
    int16_t  voltage_raw;
    int16_t  current_raw;
    uint8_t  checksum;      // XOR of bytes 0..8
};
#pragma pack(pop)
static_assert(sizeof(SampleFrame) == 10, "SampleFrame must be exactly 10 bytes");
```

**Pi Python parsing:**
```python
import struct
SYNC_BYTE = 0xAA
FRAME_SIZE = 10                    # sync(1) + timestamp(4) + voltage(2) + current(2) + checksum(1)
FRAME_FORMAT = '<BIhhB'           # <BIhhB = 1+4+2+2+1 = 10 bytes ✓

def parse_frame(raw: bytes) -> tuple | None:
    if len(raw) != FRAME_SIZE:
        return None
    sync, ts, voltage, current, chk = struct.unpack(FRAME_FORMAT, raw)
    if sync != SYNC_BYTE:
        return None
    expected_chk = 0
    for b in raw[:-1]:
        expected_chk ^= b
    if chk != expected_chk:
        return None
    return (ts, voltage, current)
```

---

## 2. Shared Memory: Ring Buffer (SampleRecord)

**Name:** `specter_ring_buffer`
**Capacity:** 4096 records × 8 bytes = **32,768 bytes**

Each record:

| Bytes | Field | Type |
|---|---|---|
| 0–3 | timestamp_us | `uint32` |
| 4–5 | voltage_raw | `int16` |
| 6–7 | current_raw | `int16` |

**Python layout:**
```python
import numpy as np
RING_CAPACITY = 4096
RING_DTYPE = np.dtype([
    ('timestamp_us', np.uint32),
    ('voltage_raw',  np.int16),
    ('current_raw',  np.int16),
])  # itemsize = 8 bytes
```

Ring buffer control (separate shared memory `specter_ring_ctrl`):
```python
CTRL_DTYPE = np.dtype([
    ('write_idx', np.uint32),   # written by Acquisition
    ('read_idx',  np.uint32),   # written by Compensation
])  # itemsize = 8 bytes
```

---

## 3. Shared Memory: Coefficient Vector

**Name:** `specter_coefficients`
**Size:** 80 × 8 bytes (float64) + 1 byte (lock flag) = **641 bytes**

Layout:
- `[0..63]` : 64 Wiener deconvolution FIR tap coefficients (float64)
- `[64..79]` : 16 dominant 2nd-order Volterra quadratic kernel coefficients (float64)

**Python:**
```python
COEFF_DIM = 80
COEFF_DTYPE = np.float64
COEFF_SIZE_BYTES = COEFF_DIM * 8 + 1  # +1 for lock flag byte

# Access pattern:
coeff_array = np.ndarray((COEFF_DIM,), dtype=np.float64, buffer=shm_coeff.buf)
lock_flag = np.frombuffer(shm_coeff.buf, dtype=np.uint8, count=1, offset=COEFF_DIM * 8)
```

---

## 4. Shared Memory: Output Buffer

**Name:** `specter_output`
**Capacity:** 256 records × 16 bytes = **4,096 bytes**

Each output record:

| Bytes | Field | Type |
|---|---|---|
| 0–3 | voltage_raw | `float32` |
| 4–7 | voltage_corrected | `float32` |
| 8–11 | current_raw | `float32` |
| 12–15 | current_corrected | `float32` |

```python
OUTPUT_CAPACITY = 256
OUTPUT_DTYPE = np.dtype([
    ('voltage_raw',       np.float32),
    ('voltage_corrected', np.float32),
    ('current_raw',       np.float32),
    ('current_corrected', np.float32),
])
```

---

## 5. Shared Memory: Telemetry Frame

**Name:** `specter_telemetry`
**Size:** 2048 bytes (JSON string, null-padded)

JSON schema:
```json
{
  "ts_ms":           1234567890,
  "state":           "COMPENSATING",
  "voltage_raw":     2.341,
  "voltage_corrected": 2.387,
  "current_raw":     0.234,
  "current_corrected": 0.239,
  "crossmodal_err":  0.0012,
  "ekf_state_norm":  0.0834,
  "cusum_value":     0.23,
  "confidence":      91,
  "bootstrap_pct":   100,
  "bispectrum":      [[0.1, 0.2, ...], [...], ...],
  "h1_mag":          [1.0, 0.98, 0.91, ...],
  "h1_phase":        [0.0, -0.01, -0.03, ...]
}
```

---

## 6. UART Telemetry (Pi → ESP32)

**Protocol:** 115200 baud 8N1, newline-delimited JSON
**Rate:** 10 Hz
**Format:** JSON string from schema above, terminated with `\n`
**Max length:** 2048 bytes

---

## 7. System State Enum

```python
# specter/state.py
from enum import Enum

class SystemState(str, Enum):
    COLD_BOOT     = "COLD_BOOT"
    BOOTSTRAP     = "BOOTSTRAP"
    COMPENSATING  = "COMPENSATING"
    RE_BOOTSTRAP  = "RE_BOOTSTRAP"
    SENSOR_SWAP   = "SENSOR_SWAP"
    ERROR         = "ERROR"
```

---

## 8. EKF State Vector (80-Dimensional)

```
x[0..63]  : w_wiener[0..63]    — Wiener FIR tap coefficients (float64)
x[64..79] : h2_dominant[0..15] — Dominant 2nd-order Volterra kernel coefficients (float64)

Initial values after bootstrap:
  x[0..63]  = output of IFFT64(W(ω))   (Wiener taps)
  x[64..79] = top-16 entries of h2 vector by |magnitude|

Process noise covariance Q:
  Q = diag([q_wiener]×64 + [q_volterra]×16)
  q_wiener   = config.ekf_Q_diagonal   (default 1e-6)
  q_volterra = config.ekf_Q_diagonal   (default 1e-6)

Initial state covariance P:
  P = 1e-4 × I(80)
```

---

## 9. Forensics Report Model

Generated after each bootstrap. Stored as JSON and plaintext.

```python
@dataclass
class ForensicsReport:
    sensor_id: str                     # "voltage" or "current"
    bootstrap_timestamp_iso: str       # ISO-8601
    confidence_score: int              # 0–100
    nonlinear_transfer_curve: dict     # {"x": [...], "y_deviation": [...]}
    frequency_response_mag: dict       # {"freq_hz": [...], "magnitude_db": [...]}
    frequency_response_phase: dict     # {"freq_hz": [...], "phase_deg": [...]}
    noise_floor_psd: dict              # {"freq_hz": [...], "psd_dBHz": [...]}
    drift_rate_per_hour: float         # estimated from EKF Q matrix
    calibration_confidence: int        # 0–100, same as above
    bispectrum_peak_freqs: list        # [(f1, f2), ...] dominant peaks
    regularization_lambda: float       # Tikhonov λ used in this bootstrap
    frames_averaged: int               # actual M used
```

---

## 10. Config Model

Loaded once at startup from `config/specter_config.yaml`.

```python
from dataclasses import dataclass

@dataclass(frozen=True)
class SpectreConfig:
    # Sampling
    voltage_sample_rate_hz: int
    current_sample_rate_hz: int
    crossmodal_rate_hz: int

    # Bootstrap
    bispectrum_fft_size: int
    bispectrum_frames_M: int
    bootstrap_timeout_s: int
    volterra_memory_taps_linear: int
    volterra_memory_taps_quadratic: int

    # Regularization
    tikhonov_lambda_volterra: float
    wiener_lambda_floor: float

    # EKF
    ekf_state_dim: int
    ekf_Q_diagonal: float
    ekf_R_initial: float
    ekf_update_rate_hz: int

    # CUSUM
    cusum_slack_factor: float
    cusum_threshold_sigma: float

    # Cross-modal
    r_load_ohms: float
    crossmodal_alpha: float
    crossmodal_beta: float
    crossmodal_gamma: float

    # Telemetry
    telemetry_rate_hz: int
    uart_baud_esp32: int

    # Hardware
    teensy_usb_path: str
    pi_uart_esp32: str

    # Forensics
    confidence_green_threshold: int
    confidence_amber_threshold: int
```
