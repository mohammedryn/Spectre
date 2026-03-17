# TASKS.md — SPECTER Build Task List

## How to Use This File

- This is the ONLY file that tracks build progress.
- Work chunks **in strict order**. Each chunk depends on the previous.
- Mark tasks: `[ ]` not started, `[/]` in progress, `[x]` done.
- **Never mark a task done without passing its validation steps.**
- After completing a chunk, run its tests before moving to the next.

---

## Chunk 0 — Scaffold (Start Here)

- [x] **C-00-A**: Create `config/specter_config.yaml` from the template in `docs/TECH_STACK.md`. Fill in your actual hardware paths and measured R_load value.
- [x] **C-00-B**: Create all Python module stubs: each file in `specter/` with module docstring + `pass`. No logic yet.
- [x] **C-00-C**: Create `tests/conftest.py` with two pytest fixtures:
  - `synthetic_sensor_signal()` — generates white Gaussian noise passed through `y = x + 0.3x² + noise` at 3200 Hz, 10 seconds
  - `mock_config()` — returns SpectreConfig loaded from `config/specter_config.yaml`
- [x] **C-00-D**: Verify `python -m pytest tests/ -v` runs (all tests skip/pass on stubs — no failures).

**Validation:** `pytest tests/ --collect-only` shows all test files collected.

---

## Chunk 1 — Teensy Firmware: Config and Scaffold

- [x] **C-01-A**: Create `firmware/teensy/platformio.ini` from template in `docs/TECH_STACK.md`.
- [x] **C-01-B**: Create `firmware/teensy/src/main.cpp` with empty `setup()` and `loop()`.
- [x] **C-01-C**: Create `firmware/teensy/src/acquisition.h` — declare constants and types:
  ```cpp
  constexpr uint32_t SAMPLE_RATE_HZ = 3200;
  constexpr uint32_t SAMPLE_PERIOD_US = 1000000 / SAMPLE_RATE_HZ;
  constexpr uint16_t BUFFER_SIZE_SAMPLES = 512;
  constexpr uint8_t  VOLTAGE_ADC_PIN = A0;
  constexpr uint8_t  SYNC_BYTE = 0xAA;
  ```
  Declare `SampleFrame` struct (see `docs/DATA_MODELS.md` section 1). Declare `ISR_sample_tick()`.
- [x] **C-01-D**: Create `firmware/teensy/src/acquisition.cpp` — stub bodies only.
- [x] **C-01-E**: Verify: `pio run` compiles without errors (empty firmware is fine).

**Validation:** PlatformIO build succeeds with 0 errors, 0 warnings.

---

## Chunk 2 — Teensy Firmware: INA219 Driver

- [x] **C-02-A**: Create `firmware/teensy/src/ina219.h` — declare:
  ```cpp
  void INA219_init();
  int16_t INA219_read_current_raw();  // returns raw ADC value, signed
  bool INA219_is_ready();
  ```
- [x] **C-02-B**: Create `firmware/teensy/src/ina219.cpp` — implement:
  - I2C address: `0x40`
  - Config register (0x00): set to fastest conversion (128 samples average, 532μs conversion)
  - `INA219_init()`: write config register, verify by reading it back
  - `INA219_read_current_raw()`: read current register (0x04), return as int16_t
  - `INA219_is_ready()`: check conversion ready bit in bus voltage register
- [x] **C-02-C**: In `main.cpp setup()`: call `Wire.begin()` at 400 kHz, call `INA219_init()`, print "INA219 OK" or "INA219 FAIL" to Serial.
- [x] **C-02-D**: In `main.cpp loop()`: print voltage raw + current raw every 500 ms.
- [x] **C-02-E**: Flash to Teensy. Open serial monitor. Verify both readings update at ~2 Hz.

**Validation (manual):** Serial prints two numeric values that change when you vary voltage/current.

---

## Chunk 3 — Teensy Firmware: Acquisition ISR + Ping-Pong Buffer

- [ ] **C-03-A**: In `acquisition.h` — declare:
  ```cpp
  extern volatile bool g_buffer_ready;
  extern volatile uint8_t g_active_buffer_idx;
  extern int16_t g_voltage_buf[2][BUFFER_SIZE_SAMPLES];
  extern int16_t g_current_buf[2][BUFFER_SIZE_SAMPLES];
  extern uint32_t g_timestamp_buf[2][BUFFER_SIZE_SAMPLES];
  void acquisition_init();
  void ISR_sample_tick();   // called by IntervalTimer
  ```
- [ ] **C-03-B**: In `acquisition.cpp` — implement `ISR_sample_tick()`:
  ```cpp
  void ISR_sample_tick() {
      // read voltage ADC and latest cached current value into inactive buffer
      // increment buffer write position
      // if position == BUFFER_SIZE_SAMPLES: swap buffers, set g_buffer_ready = true
  }
  ```
  STRICT RULES: no malloc, no Serial, no delay, no I2C transaction, < 1 μs to complete.
- [ ] **C-03-C**: In `acquisition_init()`: configure `analogReadResolution(12)`, `analogReadAveraging(1)`, start IntervalTimer at `SAMPLE_PERIOD_US`.
- [ ] **C-03-D**: In `loop()`: poll INA219 conversion-ready and update `g_latest_current_raw` (non-blocking). If `g_buffer_ready`, toggle a GPIO pin (for jitter measurement), clear `g_buffer_ready`.
- [ ] **C-03-E**: Measure GPIO toggle period with oscilloscope. Target: 3200 Hz ± 0.1 Hz, jitter < 100 ns.

**Validation (oscilloscope):** GPIO toggles at 3200 Hz, jitter < 100 ns across 1000 measurements.

---

## Chunk 4 — Teensy Firmware: USB Serial Protocol

- [ ] **C-04-A**: Create `firmware/teensy/src/usb_stream.h` — declare:
  ```cpp
  void usb_stream_init();
  void usb_stream_transmit_buffer(uint8_t buf_idx);  // transmit inactive buffer
  ```
- [ ] **C-04-B**: Create `firmware/teensy/src/usb_stream.cpp` — implement:
  - For each sample in buffer: build `SampleFrame` struct, compute XOR checksum, write 10 bytes via `Serial.write()`
  - Send `BUFFER_SIZE_SAMPLES` frames per call = 512 × 10 = 5120 bytes per call
  - At 3200 Hz with 512-sample buffer: transmit every 160 ms = 32,000 bytes/second → well within USB serial limits
- [ ] **C-04-C**: Move `loop()` logic: if `g_buffer_ready`, call `usb_stream_transmit_buffer(1 - g_active_buffer_idx)`, clear flag.
- [ ] **C-04-D**: Pi-side validation script `scripts/validate_serial.py`:
  - Open serial port
  - Read 1000 frames
  - Validate every frame: sync byte, checksum
  - Print: total frames, valid frames, bad frames, timestamp deltas
- [ ] **C-04-E**: Run validation script for 30 seconds.

**Validation:** 0 checksum failures, timestamp deltas consistent with 3200 Hz (312.5 μs ± 0.01%).

---

## Chunk 5 — Pi: Config Loader and Shared Memory Layout

- [ ] **C-05-A**: Implement `specter/config_loader.py`:
  - `load_config(path: str = "config/specter_config.yaml") -> SpectreConfig`
  - Must raise `FileNotFoundError` if file missing (not silent fail)
  - Must raise `KeyError` with field name if any required key is missing
- [ ] **C-05-B**: Implement `specter/shared_memory_layout.py`:
  - All shared memory names as constants: `SHM_RING`, `SHM_COEFFICIENTS`, `SHM_OUTPUT`, `SHM_TELEMETRY`, `SHM_HEARTBEAT`, `SHM_CTRL`
  - All sizes as constants (see `docs/DATA_MODELS.md` section 2–6)
  - All dtypes as constants
  - Helper: `create_all_shm() -> dict[str, SharedMemory]`
  - Helper: `attach_all_shm() -> dict[str, SharedMemory]`
- [ ] **C-05-C**: Write `tests/integration/test_shared_memory.py`:
  - Create shm, write known values from one thread, read from another, verify correctness
  - Test: no data corruption after 10,000 read/write cycles
- [ ] **C-05-D**: Run tests: `pytest tests/integration/test_shared_memory.py -v`

**Validation:** All shared memory tests pass. Config loads without error.

---

## Chunk 6 — Pi: Acquisition Process

- [ ] **C-06-A**: Implement `specter/acquire.py` — full process:
  ```python
  def run_acquisition(config: SpectreConfig, shm_names: dict, shutdown: Event):
      """Main function for Acquisition process (Core 0)."""
  ```
  - Open pyserial connection to Teensy USB path
  - Parse 10-byte frames using `struct.unpack` (see `docs/DATA_MODELS.md`)
  - Validate checksum — drop silently on failure, increment error counter
  - Write to shared memory ring buffer — update write pointer atomically
  - Send heartbeat: write `time.time_ns()` to `SHM_HEARTBEAT[0]` every 1 second
  - On USB disconnect: log warning, wait 2 seconds, retry
  - Exit cleanly when `shutdown` event is set
- [ ] **C-06-B**: Test with Teensy connected: run `python -m specter.acquire` as standalone. Verify ring buffer fills at correct rate.
- [ ] **C-06-C**: Write `tests/integration/test_serial_protocol.py`:
  - Mock a serial port using a bytes stream of pre-built valid frames
  - Verify all frames are parsed and written to ring buffer correctly
  - Inject 10 corrupted frames (bad checksum) — verify none written to ring buffer

**Validation:** Ring buffer fills at 3200 Hz. 0 corrupted frames written. Reconnect after USB disconnect works.

---

## Chunk 7 — Pi: Bootstrap — Bispectrum Estimation

- [ ] **C-07-A**: Implement `specter/bootstrap.py` — `estimate_bispectrum()` function:
  ```python
  def estimate_bispectrum(frames: np.ndarray, M: int,
                           fft_size: int = 512) -> np.ndarray:
      """
      Average bispectrum over M frames.
      Args:
          frames: (M, fft_size) raw signal frames, float64
          M: number of frames to average
          fft_size: FFT size (default 512)
      Returns:
          (fft_size//2, fft_size//2) complex128 bispectrum, principal domain only
      """
  ```
  Steps:
  1. Hanning window: `w = 0.5 * (1 - np.cos(2 * np.pi * np.arange(N) / N))`
  2. `rfft` per frame (use `np.fft.rfft`)
  3. Bispectrum: `B_hat[k1, k2] = X[k1] * X[k2] * np.conj(X[k1 + k2])` (principal domain only)
  4. Average over M frames
  5. Return averaged bispectrum
- [ ] **C-07-B**: Write `tests/synthetic/test_bispectrum.py`:
  - Generate: `y = x + 0.3*x**2 + noise` where `x ~ N(0,1)` at 3200 Hz, 60 seconds
  - Compute bispectrum at M=50, 100, 200
  - Assert: bispectrum has at least one peak with magnitude > 5× the median (structure present)
  - Assert: bispectrum of purely linear signal `y = x + noise` is near-zero (max < 0.05 × nonlinear case)
- [ ] **C-07-C**: Run: `pytest tests/synthetic/test_bispectrum.py -v`
- [ ] **C-07-D**: Plot convergence (in `scripts/validate_synthetic.py`): `||B_M - B_M-50||/||B_M||` vs M. Find the elbow.

**Validation:** Both unit tests pass. Convergence plot shows clear elbow by M=100.

---

## Chunk 8 — Pi: Bootstrap — Volterra Kernel Extraction

- [ ] **C-08-A**: Implement in `specter/bootstrap.py` — `estimate_H1()` and `extract_volterra_kernel()`:
  ```python
  def estimate_H1(psd: np.ndarray) -> np.ndarray:
      """Estimate linear transfer function magnitude from power spectral density."""

  def extract_volterra_kernel(bispectrum: np.ndarray, H1: np.ndarray,
                               lambda_reg: float) -> tuple[np.ndarray, np.ndarray]:
      """
      Extract H2 from bispectrum, then solve Fredholm equation for inverse kernels.
      Returns (h1_inverse_taps_64, h2_dominant_16_coeffs)
      """
  ```
  Steps for `extract_volterra_kernel`:
  1. Compute H2 = bispectrum / (H1[k1] * H1[k2] * conj(H1[k1+k2]) * Sx[k1] * Sx[k2]) where Sx is assumed white
  2. Discretize Fredholm integral to form matrix A and vector b
  3. Solve: `x = np.linalg.lstsq(A + lambda_reg * np.eye(len(A)), b, rcond=None)[0]`
  4. Check condition number of A before solve — log it
  5. Extract top-16 coefficients by magnitude for quadratic term
- [ ] **C-08-B**: Write `tests/synthetic/test_volterra.py`:
  - Synthetic: known H1 (e.g., Butterworth lowpass), f(x) = x + 0.3x² + 0.1x³
  - Run pipeline: bispectrum → H1 estimate → extract_volterra_kernel → inverse
  - Apply inverse filter to distorted signal
  - Assert: MSE of corrected output < 0.1 × MSE of raw (≥10× improvement)
- [ ] **C-08-C**: Run: `pytest tests/synthetic/test_volterra.py -v`

**Validation:** Test passes with ≥10× MSE improvement on synthetic data.

---

## Chunk 9 — Pi: Bootstrap — Wiener Deconvolution Taps

- [ ] **C-09-A**: Implement in `specter/bootstrap.py` — `compute_wiener_taps()`:
  ```python
  def compute_wiener_taps(H1: np.ndarray, noise_power: float, signal_power: float,
                           n_taps: int = 64, lambda_floor: float = 0.01) -> np.ndarray:
      """
      Compute 64-tap Wiener FIR equalizer from estimated H1.
      W(ω) = H1*(ω) / (|H1(ω)|² + λ),  λ = max(noise_power/signal_power, lambda_floor)
      Returns n_taps time-domain coefficients via IFFT.
      """
  ```
  Steps:
  1. Compute λ = max(noise_power / signal_power, lambda_floor)
  2. Evaluate W at n_taps frequency bins: `W = np.conj(H1) / (np.abs(H1)**2 + lam)`
  3. IFFT to time domain: `w = np.fft.irfft(W, n=n_taps)` → truncate to n_taps
  4. Return w (64 real-valued tap coefficients)
- [ ] **C-09-B**: Implement full `run_bootstrap()` function in `specter/bootstrap.py`:
  ```python
  def run_bootstrap(config: SpectreConfig, raw_samples: np.ndarray) -> BootstrapResult | None:
      """
      Run full bootstrap pipeline on raw_samples.
      Returns BootstrapResult or None on failure (logs error).
      """
  ```
  Calls: collect frames → `estimate_bispectrum` → `estimate_H1` → `extract_volterra_kernel` → `compute_wiener_taps` → assemble BootstrapResult
- [ ] **C-09-C**: Write `tests/synthetic/test_wiener.py`:
  - Known bandpass H1 applied to white noise
  - Run `compute_wiener_taps(H1, noise, signal)`
  - Assert output spectrum of `np.convolve(x_filtered, w)` is flatter than input: `std(|Y_out|) < 0.3 * std(|Y_in|)`
- [ ] **C-09-D**: Write `tests/synthetic/test_pipeline.py` (full integration):
  - Generate 60-second synthetic sensor signal
  - Run `run_bootstrap()` end-to-end
  - Assert BootstrapResult is not None
  - Assert wiener taps are valid (64 real floats, `np.any(np.isnan(taps)) == False`)
- [ ] **C-09-E**: Run: `pytest tests/synthetic/ -v`. All 4 test files must pass.

**Validation:** Full bootstrap pipeline passes on synthetic data. Wiener taps are real, finite, non-trivial.

---

## Chunk 10 — Pi: EKF and CUSUM

- [ ] **C-10-A**: Implement `specter/kalman.py` — `SpectreEKF` class:
  ```python
  class SpectreEKF:
      def __init__(self, config: SpectreConfig, initial_state: np.ndarray):
          """initial_state: 80-dim vector from BootstrapResult"""
      def predict(self): ...     # x = x, P = P + Q
      def update(self, z: float): ...  # z = cross-modal error ε
      def get_state(self) -> np.ndarray: ...  # returns current 80-dim x
      def get_innovation(self) -> float: ...  # latest ν_k
  ```
  Implementation notes:
  - `scipy.linalg.solve` for Kalman gain (NOT `np.linalg.inv`) — more numerically stable
  - Jacobian H (measurement matrix): start with simplified identity-based model (∂h/∂x ≈ I scaled) — document the assumption
  - Numerical stability: check `np.isfinite(P).all()` after each update — reset if diverged
- [ ] **C-10-B**: Implement `CusalDetector` class:
  ```python
  class CusumDetector:
      def __init__(self, slack_factor: float, threshold_sigma: float):
      def update(self, innovation: float) -> bool:  # True = change detected
      def reset(self):
      def get_value(self) -> float:
  ```
- [ ] **C-10-C**: Implement `run_tracking()` process function in `kalman.py`:
  - Reads cross-modal ε from shared memory at 100 Hz
  - Calls EKF predict + update
  - Writes updated state to shared memory (with lock)
  - Calls CUSUM update — if returns True: put `"CUSUM_FIRED"` in events queue
  - Sends heartbeat every 1 second
- [ ] **C-10-D**: Write `tests/synthetic/test_ekf.py`:
  - Synthetic: 80-dim state with slow linear drift in first 5 dims
  - Run EKF for 100 update steps
  - Assert: EKF estimated state converges within 5% of true drift after 50 steps
- [ ] **C-10-E**: Write `tests/synthetic/test_cusum.py`:
  - Generate stationary innovation sequence for 200 steps (no change)
  - Assert: CUSUM does NOT fire
  - Inject step change at step 200 (mean shifts by 5σ)
  - Assert: CUSUM fires within 20 steps of the change
- [ ] **C-10-F**: Run: `pytest tests/synthetic/test_ekf.py tests/synthetic/test_cusum.py -v`

**Validation:** EKF tracks drift. CUSUM fires on injected change, doesn't fire on stationary noise.

---

## Chunk 11 — Pi: Cross-Modal Consistency

- [ ] **C-11-A**: Implement `specter/crossmodal.py`:
  ```python
  def compute_crossmodal_error(v_corrected: float, i_corrected: float,
                                r_load: float) -> float:
      """Returns (v_corrected - i_corrected * r_load)**2"""

  def alternating_descent_update(v_coeffs: np.ndarray, i_coeffs: np.ndarray,
                                  v_output: np.ndarray, i_output: np.ndarray,
                                  r_load: float, gamma: float,
                                  n_iter: int = 3) -> tuple[np.ndarray, np.ndarray]:
      """
      One round of alternating coordinate descent to minimize cross-modal cost.
      Returns updated (v_coeffs, i_coeffs).
      Called after each bootstrap, not in the hot path.
      """
  ```
- [ ] **C-11-B**: Write `tests/synthetic/test_crossmodal.py`:
  - Generate V and I signals satisfying V = I × R_load + small calibration error
  - Add 3% gain error to V sensor, -2% to I sensor
  - Run alternating descent with γ=0.5
  - Assert: cross-modal error ε decreases by ≥30% after 5 iterations
- [ ] **C-11-C**: Run: `pytest tests/synthetic/test_crossmodal.py -v`

**Validation:** Cross-modal test passes. Both sensors improve simultaneously.

---

## Chunk 12 — Pi: Compensation Hot Path

- [ ] **C-12-A**: Implement `specter/compensate.py` — `run_compensation()` process:
  - Inner loop (runs at max rate, no sleep unless buffer empty):
    1. Read 512 samples from ring buffer into local frame
    2. Apply Volterra: `y_volt = np.convolve(frame, h1_inv, 'same') + quadratic_term(frame, h2_coeffs)`
    3. Apply Wiener: `z = np.dot(w_taps, y_volt[-64:])`  (sliding dot product, vectorized)
    4. Write calibrated sample to output shared memory
  - Read fresh coefficients from shared memory every 100 ms
  - Send heartbeat every 1 second
  
  Quadratic term (vectorized, numpy only):
  ```python
  def apply_volterra_quadratic(x: np.ndarray, h2: np.ndarray, L: int = 16) -> np.ndarray:
      """Apply 2nd-order Volterra term. No Python loops. Vectorized."""
      # Use np.outer, np.einsum or np.tensordot for efficiency
  ```
- [ ] **C-12-B**: Profile `run_compensation()`:
  - Measure: time per 512-sample frame
  - Target: < 5 ms per frame (comfortable budget at 3.2 kHz)
  - If > 5 ms: optimize the quadratic Volterra term (dominant cost)
- [ ] **C-12-C**: Write a standalone throughput test:
  ```bash
  python scripts/profile_pi.py
  ```
  Prints: [Volterra: X ms/frame] [Wiener: X ms/frame] [Total: X ms/frame] [Max theoretical: 160 ms]

**Validation:** Total compensation ≤ 5 ms per 512-sample frame on Pi 5.

---

## Chunk 13 — Pi: Telemetry and Forensics

- [ ] **C-13-A**: Implement `specter/forensics.py` — `generate_forensics_report()`:
  ```python
  def generate_forensics_report(bootstrap_result: BootstrapResult,
                                  config: SpectreConfig) -> ForensicsReport:
  ```
  All 5 fields (see `docs/DATA_MODELS.md` section 9). Outputs JSON and plaintext.
- [ ] **C-13-B**: Implement `specter/telemetry.py` — `build_telemetry_frame()` and `run_telemetry()`:
  - `build_telemetry_frame()`: assemble JSON dict from shared memory, return as string
  - `run_telemetry()`: calls `build_telemetry_frame()` at 10 Hz, writes to UART (pyserial)
- [ ] **C-13-C**: Test telemetry manually: run `run_telemetry()` standalone, pipe to `python -c "import sys; print(sys.stdin.readline())"` — verify valid JSON.

**Validation:** Telemetry emits valid JSON at 10 Hz. Forensics report populates all 5 fields.

---

## Chunk 14 — Pi: Orchestrator and Full Pipeline

- [ ] **C-14-A**: Implement `specter/main.py`:
  - Create all shared memory regions
  - Spawn 4 worker processes with CPU affinity (Core 0–3)
  - Run state machine at 10 Hz in a `while True` loop
  - Handle SIGINT: set `shutdown` event, join all processes, release shared memory, exit
  - Supervisor loop: check heartbeats every 3 seconds — restart dead processes
- [ ] **C-14-B**: Implement `specter/state.py` state machine logic:
  - `SystemState` enum
  - `handle_state(current: SystemState, events: dict) -> SystemState`
  - All transitions from `docs/ARCHITECTURE.md` section 5
- [ ] **C-14-C**: End-to-end smoke test: `python -m specter.main` with Teensy connected
  - System should go: COLD_BOOT → BOOTSTRAP → COMPENSATING
  - Log shows state transitions with timestamps
  - No process crashes within 60 seconds
- [ ] **C-14-D**: Run `tests/integration/test_pipeline.py`

**Validation:** Full Pi pipeline runs for 60 seconds without crash. State transitions correct.

---

## Chunk 15 — ESP32: WiFi AP and WebSocket Server

- [ ] **C-15-A**: Implement `firmware/esp32/src/wifi_ap.h/.cpp`:
  - `wifi_ap_init()`: start softAP "SPECTER-DEMO", fixed IP 192.168.4.1
  - WebSocket server on port 80
  - `broadcast_telemetry(const char* json, size_t len)`: send to all connected clients
  - Handle up to 8 simultaneous clients
- [ ] **C-15-B**: Create `firmware/esp32/src/data/index.html` — single-file dashboard:
  - Include Chart.js (minified, inline or from CDN)
  - 6 live graphs (see `docs/ARCHITECTURE.md` section on ESP32)
  - WebSocket reconnect on disconnect
  - Mobile-responsive layout
- [ ] **C-15-C**: Upload HTML to SPIFFS: `pio run -t uploadfs`
- [ ] **C-15-D**: Flash ESP32. Open browser to 192.168.4.1. Verify: dashboard loads, graphs show (empty data is fine).

---

## Chunk 16 — ESP32: UART Receiver and Display

- [ ] **C-16-A**: Implement `firmware/esp32/src/telemetry.h/.cpp`:
  - `telemetry_init()`: begin Serial2 at 115200 baud (GPIO 17/18)
  - `telemetry_tick()`: read Serial2 until `\n`, parse JSON with ArduinoJson, store in global struct, set `telemetry_updated = true`
- [ ] **C-16-B**: Implement `firmware/esp32/src/display.h/.cpp`:
  - OLED: show confidence score, system state, CUSUM value
  - LED strip: green/amber/red based on confidence and system state
- [ ] **C-16-C**: End-to-end test: Pi runs telemetry pipeline → UART → ESP32 → WebSocket → phone dashboard. All 6 graphs update at 10 Hz.

**Validation:** Dashboard updates on phone at 10 Hz. OLED shows correct state. LEDs match state.

---

## Chunk 17 — Integration: Full System

- [ ] **C-17-A**: Connect full system: Teensy → Pi → ESP32 → phone
- [ ] **C-17-B**: Run 5-act demo sequence from PRD:
  - Act 1: raw sensor drift visible on dashboard
  - Act 2: cold bootstrap completes in < 8 seconds, confidence > 85
  - Act 3: correction stable under heat stress
  - Act 4: CUSUM fires, re-bootstrap succeeds autonomously
  - Act 5: new sensor plugged in, bootstraps in < 10 seconds
- [ ] **C-17-C**: 60-minute soak test: run continuously with periodic thermal stress. Zero crashes.
- [ ] **C-17-D**: Benchmark run: `python scripts/profile_pi.py` — record all latencies.

**Validation:** All 5 demo acts pass. 60-minute soak test passes. Benchmarks match PRD claims (or document actual measured values).

---

## Chunk 18 — Validation and Documentation

- [ ] **C-18-A**: Record all accuracy measurements: raw error vs corrected error under thermal stress. Compute mean ± std over 10 trials. Save to `data/accuracy_results.csv`.
- [ ] **C-18-B**: Generate all plots: `python scripts/validate_synthetic.py` — saves PNGs to `data/plots/`.
- [ ] **C-18-C**: Write `README.md` — only claim what you have measured. Include: architecture diagram, hardware photo, getting started, measured results table.
- [ ] **C-18-D**: Record 5-7 minute demo video following 5-act structure.

**Final validation:** Every claim in README is linked to a measurement. All `pytest` tests pass.
