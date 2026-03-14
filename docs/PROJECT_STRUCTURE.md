# PROJECT_STRUCTURE.md — SPECTER

Complete directory and file map. Every file is listed with its purpose.
Do NOT create files outside this structure without updating this document.

---

## Root Layout

```
Spectre/
├── AI_RULES.md                 ← Read first. Absolute coding rules.
├── TASKS.md                    ← Build task list. Track progress here.
├── README.md                   ← Project overview (write last)
├── .gitignore
│
├── config/
│   └── specter_config.yaml     ← All tunable parameters. Never hardcode.
│
├── docs/
│   ├── SPECTER_PRD updated.md  ← Original PRD. Read-only reference.
│   ├── ARCHITECTURE.md         ← System design.
│   ├── DATA_MODELS.md          ← All data structures.
│   ├── TECH_STACK.md           ← Libraries, versions, hardware specs.
│   ├── PROJECT_STRUCTURE.md    ← This file.
│   ├── roadmap.md              ← Learning roadmap.
│   ├── issuesfaced.md          ← Engineering issues log + fixes.
│   └── laterideas.md           ← Post-baseline architecture ideas.
│
├── specter/                    ← Raspberry Pi Python pipeline
│   ├── __init__.py
│   ├── main.py                 ← Orchestrator. Spawns processes, state machine.
│   ├── acquire.py              ← Process: USB serial reader + ring buffer
│   ├── compensate.py           ← Process: Volterra + Wiener hot path
│   ├── bootstrap.py            ← Module: bispectrum + Volterra solve + Wiener taps
│   ├── kalman.py               ← Process: 80-dim EKF + CUSUM
│   ├── crossmodal.py           ← Module: Ohm's Law consistency engine
│   ├── forensics.py            ← Module: sensor characterization report
│   ├── telemetry.py            ← Module: UART → ESP32 telemetry at 10 Hz
│   ├── state.py                ← SystemState enum + transitions
│   ├── config_loader.py        ← Loads specter_config.yaml → SpectreConfig dataclass
│   └── shared_memory_layout.py ← All shared memory names, sizes, dtypes
│
├── firmware/
│   ├── teensy/
│   │   ├── platformio.ini
│   │   └── src/
│   │       ├── main.cpp        ← Setup + loop
│   │       ├── acquisition.h   ← Timer ISR + ping-pong buffer
│   │       ├── acquisition.cpp
│   │       ├── ina219.h        ← INA219 I2C driver
│   │       ├── ina219.cpp
│   │       ├── usb_stream.h    ← Binary framing + USB serial transmit
│   │       └── usb_stream.cpp
│   │
│   └── esp32/
│       ├── platformio.ini
│       └── src/
│           ├── main.cpp        ← Setup + loop
│           ├── wifi_ap.h       ← WiFi AP + WebSocket server
│           ├── wifi_ap.cpp
│           ├── telemetry.h     ← UART receiver + JSON parser
│           ├── telemetry.cpp
│           ├── display.h       ← OLED + LED strip
│           ├── display.cpp
│           └── data/
│               └── index.html  ← Dashboard HTML (single file, SPIFFS)
│
├── tests/
│   ├── __init__.py
│   ├── conftest.py             ← pytest fixtures: synthetic signals, mock config
│   ├── synthetic/
│   │   ├── test_bispectrum.py  ← Unit: bispectrum on known synthetic nonlinearity
│   │   ├── test_volterra.py    ← Unit: Volterra kernel extraction + inverse
│   │   ├── test_wiener.py      ← Unit: Wiener deconvolution tap computation
│   │   ├── test_ekf.py         ← Unit: EKF predict/update cycle
│   │   ├── test_cusum.py       ← Unit: CUSUM fires on injected change
│   │   ├── test_crossmodal.py  ← Unit: cross-modal consistency error
│   │   └── test_pipeline.py    ← Integration: full bootstrap → compensate pipeline
│   └── integration/
│       ├── test_serial_protocol.py   ← Test Teensy frame parsing
│       └── test_shared_memory.py     ← Test shm coefficient handoff
│
├── scripts/
│   ├── validate_synthetic.py   ← Run full pipeline on synthetic data, produce plots
│   ├── profile_pi.py           ← Benchmark each pipeline stage on Pi
│   ├── generate_test_signal.py ← Create synthetic sensor signals for testing
│   └── plot_forensics.py       ← Plot the forensics report from a JSON file
│
└── data/
    └── logs/                   ← Runtime logs (gitignored)
```

---

## Module Responsibilities (Quick Reference)

### `specter/main.py`
- Creates all shared memory regions (names, sizes from `shared_memory_layout.py`)
- Spawns 4 worker processes with CPU affinity pinning
- Runs the state machine in a loop (~10 Hz tick)
- Routes events: CUSUM fire → trigger bootstrap in services process
- Monitors heartbeats: restart dead processes within 3 seconds
- Handles SIGINT for graceful shutdown

### `specter/acquire.py`
- Runs as an asyncio coroutine on Core 0
- Opens `/dev/ttyACM0` (or configured path) at startup
- Reads 10-byte frames in a tight loop
- Validates checksum, discards corrupted frames
- Writes valid samples to shared memory ring buffer
- Updates ring buffer write pointer atomically
- Sends heartbeat every 1 second
- On USB disconnect: log, clear ring buffer, retry every 2 seconds

### `specter/compensate.py`
- Runs as a dedicated process on Core 1 (via CPU affinity)
- Reads samples from ring buffer (blocking read, sleeps 0.1 ms when buffer empty)
- Accumulates 512-sample frames
- On each frame: apply Volterra compensation (vectorized NumPy)
- Apply Wiener FIR equalizer (dot product with 64-tap w vector from EKF)
- Write calibrated output to shared memory output buffer
- Read fresh EKF coefficients from shared memory every update under `multiprocessing.Lock`

### `specter/bootstrap.py`
**NOT a process** — called from the Services process on demand.
```python
def run_bootstrap(config: SpectreConfig, ring_buf: np.ndarray, M: int) -> BootstrapResult:
    """Returns BootstrapResult containing all coefficients."""
```
- Steps: window → FFT → bispectrum → average → H1 estimate → H2 extraction →
  Fredholm solve (Tikhonov) → Wiener W(ω) → IFFT64 → return
- Pure NumPy. No I/O inside this function.

### `specter/kalman.py`
- Runs as a dedicated process on Core 2 (via CPU affinity)
- EKF predict/update at 100 Hz
- Reads cross-modal ε from shared memory output
- Writes updated coefficient vector to shared memory coefficients
- Accumulates CUSUM: if threshold exceeded, sends CUSUM_FIRED event to main via Queue
- Persists state to `data/logs/ekf_state.json` on each CUSUM trigger

### `specter/crossmodal.py`
**NOT a process** — called from within the Tracking process (kalman.py).
```python
def compute_crossmodal_error(v_corr: np.ndarray, i_corr: np.ndarray,
                              r_load: float) -> float:
    """Returns ||V_corr - I_corr * R_load||^2."""
```

### `specter/telemetry.py`
**NOT a process** — called from within the Services process at 10 Hz.
```python
def build_telemetry_frame(state: SystemState, output_buf: np.ndarray,
                           coeff_buf: np.ndarray, ...) -> str:
    """Returns JSON string for UART transmission."""
```

---

## File Naming Conventions

| Category | Convention | Example |
|---|---|---|
| Python modules | snake_case | `bootstrap.py` |
| Python tests | `test_` prefix | `test_bispectrum.py` |
| C++ headers | snake_case + `.h` | `acquisition.h` |
| C++ sources | snake_case + `.cpp` | `acquisition.cpp` |
| Config files | snake_case + `.yaml` | `specter_config.yaml` |
| Log files | `{module}_{date}.log` | `bootstrap_20250314.log` |
| Data files | `{sensor}_{run_id}.csv` | `voltage_run_001.csv` |

---

## .gitignore

```gitignore
# Python
.venv/
__pycache__/
*.pyc
*.pyo
*.egg-info/
.pytest_cache/
dist/
build/

# PlatformIO
firmware/teensy/.pio/
firmware/esp32/.pio/

# Data (too large for git)
data/logs/
data/*.csv
data/*.npy

# IDE
.vscode/
.idea/
*.swp
```
