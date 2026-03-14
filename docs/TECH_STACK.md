# TECH_STACK.md — SPECTER

All libraries, versions, and install commands. Exact. No approximations.

---

## Raspberry Pi 5 — Python Environment

**OS:** Raspberry Pi OS (64-bit, Bookworm) or Ubuntu 24.04 LTS ARM64
**Python:** 3.11 (system default on Bookworm 2024+)

### Python Dependencies (requirements.txt)

```
numpy==1.26.4
scipy==1.12.0
pyserial==3.5
pyyaml==6.0.1
psutil==5.9.8
pytest==8.1.0
```

### Install

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### Key Library Usage (what and why)

| Library | Used For | Key Functions |
|---|---|---|
| `numpy` | All DSP computation, array ops, FFT | `np.fft.rfft`, `np.linalg.solve`, `np.linalg.lstsq`, `@` operator |
| `scipy` | Robust linear algebra (LAPACK-backed), signal utilities | `scipy.linalg.solve`, `scipy.signal.welch` |
| `multiprocessing` | True multi-core parallelism (stdlib) | `Process`, `shared_memory`, `Lock`, `Queue`, `Event` |
| `pyserial` | USB serial read from Teensy | `serial.Serial`, `serial.read()`, `struct.unpack` |
| `pyyaml` | Config file parsing | `yaml.safe_load` |
| `psutil` | Process monitoring, CPU affinity | `psutil.cpu_affinity`, `psutil.virtual_memory` |
| `logging` | All logging (stdlib) | `logging.getLogger`, `RotatingFileHandler` |
| `struct` | Binary packet parsing (stdlib) | `struct.unpack('<BIhhB', raw)` — matches canonical 10-byte frame |
| `json` | Telemetry serialization (stdlib) | `json.dumps`, `json.loads` |
| `time` | High-resolution timing (stdlib) | `time.perf_counter_ns` |

### NOT Allowed on Pi
- `matplotlib` — in production pipeline. Allowed only in `scripts/` and `tests/`. Never import in `specter/` modules.
- `tensorflow`, `torch`, `sklearn` — ML is forbidden (requires reference labels)
- `asyncio` for compute-heavy workers — only allowed in `acquire.py` (I/O bound)

---

## Teensy 4.1 — Embedded Toolchain

**Framework:** Arduino (via PlatformIO)
**Board:** `teensy41`
**C++ Standard:** C++17

### platformio.ini

```ini
[env:teensy41]
platform = teensy
board = teensy41
framework = arduino
build_flags =
    -std=c++17
    -O2
    -Wall
    -DUSB_SERIAL
monitor_speed = 115200
upload_protocol = teensy-cli
```

### Key Teensy Libraries

| Library | Source | Used For |
|---|---|---|
| `Arduino.h` | built-in | `digitalWriteFast`, `analogRead`, `analogReadResolution`, `micros`, `Serial` |
| `SPI.h` | built-in | **Not used for voltage sensor**. Included if external SPI peripherals are added later. |
| `Wire.h` | built-in | I2C communication to INA219 |
| `IntervalTimer` | built-in (Teensyduino) | Precision hardware-timer-driven ISR |
| `DMAChannel.h` | built-in (Teensyduino) | DMA-backed SPI transfers |
| `imxrt.h` | built-in | ARM_DWT_CYCCNT for profiling |

### C++ Standard Library (embedded-safe subset)

- `<stdint.h>` — fixed-width types (`uint8_t`, `int16_t`, `uint32_t`)
- `<string.h>` — `memcpy`, `memset`
- `<math.h>` — `abs`, `sqrt` (single precision)
- **NOT allowed:** `<iostream>`, `<vector>`, `<string>` (heap-allocating containers), `<new>`, `<malloc>`

---

## ESP32-S3 — Embedded Toolchain

**Framework:** Arduino (via PlatformIO)
**Board:** `esp32-s3-devkitc-1` (or `esp32s3box` for box3 variant)
**C++ Standard:** C++17

### platformio.ini

```ini
[env:esp32s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
board_build.partitions = huge_app.csv
build_flags =
    -std=c++17
    -DBOARD_HAS_PSRAM
    -mfix-esp32-psram-cache-issue
monitor_speed = 115200
upload_speed = 921600
```

### ESP32-S3 Libraries

| Library | Source | Used For |
|---|---|---|
| `WiFi.h` | built-in ESP32 Arduino | `WiFi.softAP()` access point mode |
| `ESPAsyncWebServer` | `me-no-dev/ESPAsyncWebServer@^1.2.3` | HTTP + WebSocket server |
| `AsyncTCP` | `me-no-dev/AsyncTCP@^1.1.1` | Async TCP (dependency of ESPAsyncWebServer) |
| `ArduinoJson` | `bblanchon/ArduinoJson@^7.0.4` | JSON serialization/deserialization |
| `Adafruit_SSD1306` | `adafruit/Adafruit SSD1306@^2.5.7` | OLED 128×64 display |
| `Adafruit_GFX` | `adafruit/Adafruit GFX Library@^1.11.9` | Graphics (dependency of SSD1306) |
| `Adafruit_NeoPixel` | `adafruit/Adafruit NeoPixel@^1.12.0` | WS2812B LED strip |
| `SPIFFS.h` | built-in ESP32 | Flash filesystem for dashboard HTML |

---

## Hardware Specifications

### Teensy 4.1
- CPU: ARM Cortex-M7 @ 600 MHz
- FPU: double-precision hardware
- SRAM: 1 MB (512 KB DTCM + 512 KB OCRAM)
- Flash: 8 MB
- ADC: 12-bit SAR, up to ~800 kSPS

### Raspberry Pi 5 (8 GB)
- CPU: Broadcom BCM2712, 4× Cortex-A76 @ 2.4 GHz
- RAM: 8 GB LPDDR4X
- USB: 2× USB 3.0 A (for Teensy connection)
- UART: via GPIO pins 14 (TX) / 15 (RX) for ESP32

### ESP32-S3
- CPU: Xtensa LX7 dual-core @ 240 MHz
- RAM: 512 KB internal SRAM + 8 MB PSRAM (box3 variant)
- WiFi: 802.11 b/g/n (2.4 GHz)
- UART for Pi: UART2 (GPIO 17 TX / 18 RX)

### Sensors
- Voltage: 25V resistive divider module (~₹20) — analog output
- Current: INA219 DC current sensor — I2C, 12-bit, addr 0x40

---

## Critical Sample Rate Note

| Sensor | Max Rate | Interface |
|---|---|---|
| Voltage divider | ~800 kSPS (Teensy SAR ADC limit) | `analogRead()` on Teensy pin A0 — **no SPI** |
| INA219 current | **3.2 kHz max** (continuous conversion, I2C limited) | Teensy I2C (Wire) |
| **System sample rate** | **3,200 Hz** (synchronized pair, limited by INA219) | — |

Both sensors run at **3,200 Hz** (one synchronized pair per ISR tick). The Teensy timer ISR fires at 3.2 kHz: voltage is read via `analogRead(A0)` and the latest cached INA219 current sample is latched into the frame buffer. INA219 I2C reads occur outside the ISR in the main loop (non-blocking conversion-ready poll), preserving deterministic ISR timing.

The bispectrum frame is 512 samples at 3.2 kHz = **160 ms per frame**. 100 frames = 16 seconds bootstrap window.
Use two operating profiles:
- **Accuracy profile (default):** `M = 100` (16 s bootstrap, lower estimator variance)
- **Demo-speed profile:** `M = 50` (8 s bootstrap, higher variance but acceptable with physics prior and EKF refinement)

---

## Config File

`config/specter_config.yaml` — all tunables. Never hardcode these in Python.

```yaml
# Sample rates — both sensors synchronized at 3200 Hz
voltage_sample_rate_hz: 3200   # analogRead(A0) latched in timer ISR
current_sample_rate_hz: 3200   # INA219 continuous conversion; latest completed sample cached outside ISR
crossmodal_rate_hz: 3200        # V=IR check runs at sample rate

# Bootstrap parameters
bispectrum_fft_size: 512
bispectrum_frames_M: 100
bootstrap_timeout_s: 30
volterra_memory_taps_linear: 64
volterra_memory_taps_quadratic: 16

# Regularization
tikhonov_lambda_volterra: 1.0e-4
wiener_lambda_floor: 0.01

# EKF parameters
ekf_state_dim: 80
ekf_Q_diagonal: 1.0e-6   # process noise per state per update
ekf_R_initial: 1.0e-4    # measurement noise (overwritten after bootstrap)
ekf_update_rate_hz: 100

# CUSUM parameters
cusum_slack_factor: 1.0    # slack = cusum_slack_factor × σ_innovation
cusum_threshold_sigma: 5.0 # threshold = N × σ_innovation

# Cross-modal
r_load_ohms: 10.0          # precise resistor value — MEASURE before use
crossmodal_alpha: 1.0
crossmodal_beta: 1.0
crossmodal_gamma: 0.5

# Telemetry
telemetry_rate_hz: 10
uart_baud_esp32: 115200

# Hardware
teensy_usb_path: /dev/ttyACM0
pi_uart_esp32: /dev/ttyS0

# Forensics
confidence_green_threshold: 85
confidence_amber_threshold: 70
```
