# AI_RULES.md — SPECTER Project
## Mandatory Rules for AI-Assisted Development

Read this file before touching any other file. These rules are non-negotiable.

---

## 1. Source of Truth Hierarchy

When documents conflict, this is the priority order:

1. **AI_RULES.md** (this file) — highest authority
2. **docs/ARCHITECTURE.md** — system design and boundaries
3. **docs/DATA_MODELS.md** — all data structures and protocols
4. **docs/TECH_STACK.md** — exact libraries and versions
5. **docs/SPECTER_PRD updated.md** — product requirements and background
6. **TASKS.md** — ordered build tasks

---

## 2. What SPECTER Is (Never Forget This)

SPECTER is a **real-time, reference-free, self-calibrating signal processing pipeline** for commodity sensors with completely unknown characteristics. It operates on three nodes:

- **Teensy 4.1** — deterministic real-time sensor acquisition and hot-path DSP
- **Raspberry Pi 5** — bootstrap math (bispectrum, Volterra, Wiener), EKF, orchestration
- **ESP32-S3** — live WiFi dashboard for demo

The core algorithm chain is:
```
Raw samples → Bispectrum (blind ID) → Volterra inverse (nonlinearity correction)
→ Wiener deconvolution (frequency correction) → EKF (drift tracking)
→ Cross-modal consistency (physics anchor) → Calibrated output
```

---

## 3. Absolute Hard Rules — Never Violate

### 3.1 No Reference Signals
SPECTER is **blind**. It must NEVER use:
- A reference sensor for comparison
- Manufacturer datasheet values
- Pre-loaded calibration tables
- Ground truth labels of any kind during operation

Any code that imports external calibration data violates the core premise.

### 3.2 Language Boundaries
| Node | Language | Reason |
|---|---|---|
| Raspberry Pi (hot path: compensate.py) | Python + NumPy vectorized | GIL-safe via separate process; NumPy C backend |
| Raspberry Pi (all other modules) | Python 3.11 | Consistency |
| Teensy 4.1 firmware | C++ (Arduino framework) | Real-time, bare-metal |
| ESP32-S3 firmware | C++ (Arduino framework) | WebSocket + WiFi stack |
| Dashboard UI | HTML + vanilla JS + Chart.js | Single-file, served from ESP32 flash |

**Never use Python for Teensy or ESP32 code.**
**Never use C++ on the Raspberry Pi.**

### 3.3 Pure NumPy Hot Path
In `specter/compensate.py`, zero Python `for` loops are allowed in the compensation code path. All operations must be vectorized NumPy. If you cannot write it without a loop, you are doing it wrong.

### 3.4 Shared Memory for Inter-Process Data
Data shared between Pi processes uses `multiprocessing.shared_memory` exclusively.
- Do **NOT** use `multiprocessing.Queue` for high-rate sample data
- Queue is only for control signals and telemetry (low rate)
- High-rate sample data: shared memory ring buffer
- Coefficient vectors: shared memory, protected by `multiprocessing.Lock`

### 3.5 No Hardcoded Constants
All tunable parameters live in `config/specter_config.yaml`.
No magic numbers in code. Every constant must have a comment explaining its derivation.

### 3.6 Deterministic ISRs on Teensy
Teensy ISRs (Interrupt Service Routines) must:
- Contain ZERO dynamic memory allocation (`new`, `malloc`)
- Contain ZERO `Serial.print()` or any blocking call
- Toggle a flag or copy into a buffer, nothing else
- Complete in under 1 microsecond

### 3.7 Checksum on Every Serial Packet
Every packet from Teensy to Pi carries an XOR checksum. Pi discards corrupted packets silently. Never act on corrupted data.

---

## 4. Code Style Rules

### Python
- PEP 8 compliant
- Type hints on every function signature
- Every module has a module-level docstring explaining its role in the pipeline
- Every function has a docstring with: purpose, args, returns, raises
- Use `logging` — never `print()` in production code
- Log levels: `DEBUG` for per-sample data, `INFO` for state transitions, `WARNING` for degraded operation, `ERROR` for failed operations

```python
# CORRECT — type hints, docstring, logging
def estimate_bispectrum(frames: np.ndarray, M: int) -> np.ndarray:
    """
    Estimate the averaged bispectrum over M frames.

    Args:
        frames: (M, N) array of windowed signal frames, complex64
        M: number of frames to average
    Returns:
        (N//2, N//2) complex128 bispectrum estimate
    Raises:
        ValueError: if frames.shape[0] < M
    """
    ...
```

### C++ (Teensy / ESP32)
- C++17, `#pragma once` for all headers
- All embedded types use `stdint.h` fixed-width types: `uint8_t`, `int16_t`, `uint32_t`
- No `String` class on Teensy — use `char[]` and `snprintf()`
- Every ISR function prefixed with `ISR_`
- Every DMA callback prefixed with `DMA_`
- Volatile keyword on ALL variables shared between ISR and main loop

```cpp
// CORRECT
volatile bool g_buffer_ready = false;  // set in ISR, read in loop()
static int16_t g_sample_buffer_a[BUFFER_SIZE_SAMPLES];  // never ISR_alloc
```

---

## 5. Build and Test Rules

### Synthetic Before Real Hardware
Every DSP algorithm must pass a synthetic data test before running on real sensor data.
- Generate known distortion mathematically
- Apply algorithm
- Verify recovery meets criteria
- Only then connect real hardware

### Test File Naming
```
tests/synthetic/test_bispectrum.py   # unit test for bispectrum module
tests/synthetic/test_volterra.py     # unit test for Volterra
tests/synthetic/test_wiener.py
tests/synthetic/test_ekf.py
tests/integration/test_pipeline.py  # end-to-end pipeline test
```

### Running Tests
```bash
python -m pytest tests/ -v
```
All tests must pass before any code is pushed to repo.

---

## 6. File Modification Rules

- Never modify `AI_RULES.md` during coding — it is a constraint document
- Never modify `docs/SPECTER_PRD updated.md` — it is a reference document
- `TASKS.md` is the ONLY file that tracks build progress — update it when tasks complete
- Never create files outside the project structure defined in `docs/PROJECT_STRUCTURE.md`

---

## 7. Error Handling Philosophy

- **Never silently swallow exceptions** — always log the error before recovering
- Use specific exception types — never bare `except:`
- Failed bootstrap: log, fall back to last-known-good coefficients, continue
- Lost Teensy USB connection: log, keep ring buffer frozen, retry reconnect every 2 seconds
- EKF divergence detected (state norm > threshold): reset to bootstrap initial state, log warning
- Any unhandled exception in a worker process: the supervisor restarts it within 3 seconds

---

## 8. What You Are Building (Task Decomposition Reference)

See `TASKS.md` for the complete ordered task list. The high-level chunks are:

| Chunk | What It Is |
|---|---|
| **C-01 to C-04** | Teensy firmware: acquisition, DMA, serial protocol |
| **C-05 to C-07** | Pi acquisition pipeline: serial reader, ring buffer, logging |
| **C-08 to C-10** | Bootstrap: bispectrum, Volterra kernel, Wiener taps |
| **C-11 to C-12** | Compensation hot path: Volterra filter, Wiener filter |
| **C-13 to C-15** | Tracking: EKF, CUSUM, cross-modal |
| **C-16 to C-17** | Services: telemetry, forensics report |
| **C-18 to C-19** | Orchestration: state machine, supervisor, shared memory layout |
| **C-20 to C-22** | ESP32 firmware: WiFi AP, WebSocket server, dashboard HTML |
| **C-23 to C-25** | Integration: end-to-end test, stress test, benchmarks |

Work chunks in order. Do NOT skip ahead.
