# SPECTER — Issues Faced & Resolved

**Project:** SPECTER (Self-calibrating Pipeline for Estimating Characteristics of Totally Unknown Embedded Sensors in Real-time)
**Competition:** HW-2 Hardware Track · Spectre 2025 · BMSIT
**Document Purpose:** Chronological log of technical issues identified in the PRD, their root causes, consequences, and resolutions.

---

## Issue Log

---

### ISSUE-001 — Physically Unjustifiable Cross-Modal Sensor Pair

| Field | Detail |
|---|---|
| **Date** | March 13, 2026 |
| **Time** | Session 1 |
| **Severity** | High — directly undermines the theoretical validity of a core algorithmic layer |
| **Status** | ✅ Resolved |

#### Problem Description

The original PRD defined the cross-modal sensor pair as an **ADXL345 3-axis accelerometer** and a **BMP388 barometric pressure sensor**. These two sensors were specified as the physical anchor for the cross-modal consistency engine in Section 4.6 — the layer that uses inter-sensor physics to lock calibration across both sensors simultaneously.

#### Root Cause

The cross-modal consistency engine works by exploiting a **physical law** that constrains the two sensor outputs to satisfy a specific mathematical relationship. The correctness and tightness of the calibration anchor depends entirely on the **strength and exactness of that physical law**.

The accelerometer measures inertial force (g); the pressure sensor measures ambient barometric pressure (Pa). These two quantities are related only extremely weakly and indirectly — in a bench demo environment, they are effectively uncorrelated. There is no clean first-principles equation of the form `f(a_output, p_output) = constant` that holds under controlled demo conditions. Any attempt to formulate `J_cross` would require empirical coupling coefficients that are noisy, nonlinear, and environment-dependent. In a vibration-free lab environment (typical hackathon setting), the accelerometer output is near-zero DC with no meaningful variation, making the cross-modal signal degenerate.

#### Consequence if Left Unresolved

- The cross-modal engine would effectively be applying a penalty term with no physical basis
- `J_cross` would be fitting to measurement noise rather than a true physical constraint
- The joint calibration would potentially *degrade* accuracy rather than improve it
- Any technically informed judge would immediately identify the lack of physical coupling as a fundamental flaw
- The ~40% accuracy improvement claim attributed to cross-modal enforcement (Wow Factor 5) would be undemonstrable and likely false

#### Fix Applied

Replaced the sensor pair with a **25V resistive voltage divider module (~₹20)** and an **INA219 DC current sensor (~₹150, I2C interface)**. These two sensors measure across the same load resistor, coupling them through **Ohm's Law**:

```
V = I × R_load
```

This is an exact physical law — not an approximation, not empirical, not environment-dependent. Under any conditions where R_load is stable (a fixed precision resistor), the voltage sensor output and current sensor output **must** satisfy this relationship to arbitrary precision. The cross-modal consistency cost now has a rigorous, first-principles foundation:

```
J_cross = ||V_corrected − I_corrected × R_load||²
```

The coupling is:
- **Exact:** No approximation involved. Any nonzero J_cross is definitively attributable to calibration error, not to a weak or noisy physical relationship.
- **Easy to demonstrate:** Vary load voltage or swap R_load values mid-demo — both traces update predictably.
- **Independently drifting sensors:** Resistor temperature drift affects the voltage sensor gain; shunt resistance drift affects the INA219 gain. They drift via different mechanisms, providing genuinely independent constraints.
- **Cheap and available:** Total cost under ₹200, sourced at any electronics shop.

#### Sections Updated in PRD

- Section 3 (Wow Factor 5) — sensor pair description and coupling law
- Section 4.6 — Cross-Modal Objective Function: `J(θ_v, θ_i)` with explicit `V = I × R_load`
- Section 5.5 — Sensors table: ADXL345 + BMP388 → voltage divider + INA219
- Section 6.1 — Architecture diagram sensor line
- Section 7.2 Step 5 — Consistency error formula: `z_voltage − z_current × R_load`
- Section 8.2 — Teensy USB frame format: `voltage_raw:2B, current_raw:2B` + I2C note
- Section 9 Act 1 — Demo narration updated to power monitoring domain
- Section 10 — Risk table: cross-modal fallback updated to precision R_load guidance
- Section 11 Hours 25–28 — Build timeline updated to R_load measurement + Ohm's Law verification

---

### ISSUE-002 — Constant Modulus Algorithm (CMA) Applied to Sensor Data

| Field | Detail |
|---|---|
| **Date** | March 13, 2026 |
| **Time** | Session 2 |
| **Severity** | Critical — the algorithm is fundamentally inapplicable to the signal type; it will produce incorrect results and potentially introduce new distortion |
| **Status** | ✅ Resolved |

#### Problem Description

The original PRD specified the **Constant Modulus Algorithm (CMA)** as the blind equalization layer (Section 4.4) — a 64-tap adaptive FIR equalizer with the update rule:

```
w(n+1) = w(n) − μ · e(n) · z(n) · y*(n)
where  e(n) = |z(n)|² − R₂
```

This algorithm was described as operating on sensor signals to remove frequency-dependent gain and phase errors introduced by the sensor's linear dynamic filter H₁(ω).

#### Root Cause

CMA is a blind adaptive equalization technique originally developed for **digital communications** — specifically for demodulating QAM (Quadrature Amplitude Modulation) signals transmitted over unknown wireless or cable channels. It works because QAM symbols are drawn from a finite constellation where **every symbol has exactly the same power**: `|s(n)|² = R₂ = constant`. This is the defining property of constant-modulus signals.

CMA's error signal `e(n) = |z(n)|² − R₂` measures how far the equalizer output deviates from this constant power. By minimising this error iteratively, the equalizer converges to the inverse of the channel — because the *only* filter that maps constant-modulus input to constant-modulus output (given the distorted received signal) is the channel inverse.

**Physical sensor signals do not have constant modulus.** A voltage sensor measuring a circuit node reads 0.05V at idle, 1.2V under light load, 4.7V under heavy load. The output power `|z(n)|²` varies continuously across orders of magnitude. There is no `R₂` value that represents the "correct" output power.

The consequences of applying CMA to sensor data:
1. CMA's gradient descent is minimising the wrong objective — it is trying to flatten the output power, not invert the sensor's frequency response
2. `R₂` estimated from bootstrap statistics is the mean output power, which is a meaningless target for a signal with a dynamic range
3. On non-stationary signals (which all real-world sensor signals are), CMA's iterative updates can diverge or converge to a solution that actively distorts the calibrated output
4. The step size μ = 0.001, regardless of how conservatively chosen, does not resolve the fundamental inapplicability of the algorithm — it only slows an incorrect convergence

#### Consequence if Left Unresolved

- The equalizer, once "converged," would be applying a filter derived from a meaningless optimisation criterion — the signal's mean power level
- This would suppress frequency bands where the sensor signal has lower power (likely high-frequency components) — the exact opposite of correct equalization for a sensor with high-frequency roll-off
- The calibrated output `z[n]` could be *less accurate* than the Volterra-compensated output `x_volt` that feeds it
- Any signal processing expert on the judging panel would immediately identify CMA as inapplicable and use it to question the entire pipeline's theoretical validity

#### Fix Applied

Replaced CMA with **Blind Wiener Deconvolution** — an analytically optimal, non-iterative equalizer derived directly from the H₁(ω) estimate already computed during the bootstrap phase.

The equalizer tap coefficients in the frequency domain are:

```
W(ω) = H₁*(ω) / (|H₁(ω)|² + λ)
```

Where:
- `H₁(ω)` is the linear frequency response of the sensor, already estimated from the bispectrum and power spectrum during bootstrap
- `λ = σ²_noise / σ²_signal` is the noise-to-signal power ratio, estimated from bootstrap frame statistics
- `W(ω)` is evaluated at 64 frequency bins; the 64 time-domain FIR tap coefficients are the 64-point IFFT of W(ω)

This is the classical Wiener filter for channel inversion. Its mathematical properties are:
- **Minimises mean-squared error (MSE)** between equalizer output and true input — this is the optimal linear estimator
- **λ regularisation** prevents noise amplification at frequency bins where H₁(ω) was estimated poorly (near-zero magnitude) — the filter rolls off gracefully rather than dividing by near-zero
- **No iterative update, no step size, no convergence** — computed once in under 0.5 ms on the Pi 5 during bootstrap
- **Valid for arbitrary signal statistics** — the derivation makes no assumption about signal power distribution, modulus, or stationarity
- **No local minima** — the solution is unique and globally optimal

#### Architectural Elegance: EKF Eliminates the Adaptive Loop Entirely

The fix achieves more than just correctness — it simplifies the architecture. The 64 linear tap coefficients of the equalizer are part of the EKF's 80-dimensional state vector (64 linear taps + 16 dominant quadratic Volterra coefficients). The EKF was already tracking the evolution of `H₁(ω)` over time as the sensor drifts. By placing the equalizer taps inside the EKF state:

- Every EKF update at 100 Hz automatically recomputes and pushes new equalizer coefficients to the hot path
- The equalizer continuously tracks sensor drift without any separate adaptive mechanism
- The hot path (`compensate.py`) only reads the current coefficient vector — it does nothing iterative
- CMA's separate adaptation loop and all its tuning parameters (μ, R₂, stability check) are eliminated entirely

What was previously a second adaptive subsystem is now a zero-cost by-product of the EKF that was already required. The pipeline is both more correct and structurally simpler.

#### Implementation Change (Bootstrap Step 8)

**Before:**
```
Initialise CMA equalizer with identity response. Estimate the Godard constant R₂ from frame statistics.
```

**After:**
```
Compute the Blind Wiener Deconvolution equalizer tap vector. Estimate σ²_noise from Volterra-compensated
residual; estimate σ²_signal from frame statistics. Compute λ = σ²_noise / σ²_signal.
Evaluate W(ω) = H₁*(ω) / (|H₁(ω)|² + λ) at 64 frequency bins. Take 64-point IFFT to obtain
time-domain tap vector w[0..63]. Write w as the initial linear component of the EKF state vector.
Runtime: under 0.5 ms on Pi 5.
```

#### Implementation Change (Online Compensation Step 3)

**Before:**
```
Feed x_volt into the CMA equalizer. Compute z[n] = w^T · x_volt_buffer.
Update tap weights: w ← w − μ·(|z|²−R₂)·z·x_volt*. Step size μ = 0.001.
```

**After:**
```
Feed x_volt into the Wiener deconvolution equalizer. Compute z[n] = w^T · x_volt_buffer,
where w is the current 64-tap coefficient vector drawn directly from the EKF state.
No iterative tap update occurs in the hot path — w is updated only when the EKF pushes
a new state estimate at 100 Hz.
```

#### Sections Updated in PRD

- Section 1 (Executive Summary) — Algorithm description updated
- Section 4.4 — Entire section rewritten: "Constant Modulus Algorithm" → "Blind Wiener Deconvolution"; full derivation and justification added, including explicit statement of why CMA is inapplicable to sensor signals
- Section 6.1 — Architecture diagram: "CMA BLIND EQUALIZER" → "WIENER DECONV EQUALIZER"
- Section 6.2 — State machine table: CMA references updated
- Section 7.1 Step 8 — Bootstrap procedure: tap computation method updated
- Section 7.2 Step 3 — Online compensation: iterative update removed
- Section 7.4 — Forensics report: "after Volterra and CMA correction" → "after Volterra and Wiener deconvolution correction"; confidence score metric updated
- Section 8.1 (`compensate.py`) — Module description updated with EKF state coupling explanation
- Section 8.1 (`bootstrap.py`) — Module description updated
- Section 5.3 (Pi 5 role) — Algorithm stack description updated
- Section 10 — Risk table: "CMA equalization divergence" row replaced with "Wiener equalizer noise amplification at low SNR" row with λ-clamping mitigation
- Section 11 Hours 18–21 — Build timeline updated to Wiener deconvolution implementation
- Section 12.2 — Competitive differentiation table: algorithm list updated
- Section 14 (Glossary) — "CMA" entry replaced with "Wiener Deconvolution" entry with full definition

---

### ISSUE-003 — Fredholm Kernel Extraction Numerically Unstable Without Regularization

| Field | Detail |
|---|---|
| **Date** | March 13, 2026 |
| **Time** | Session 3 |
| **Severity** | High — produces oscillatory or exploding Volterra kernel coefficients; directly corrupts inverse filter quality |
| **Status** | ✅ Resolved |

#### Problem Description

Section 7.1 Step 7 of the original PRD described the Volterra kernel extraction as:

> *"Compute the Volterra inverse filter by solving the 2D Fredholm equation of the second kind, discretised into a linear system (`scipy.linalg.solve`)."*

No mention of conditioning, regularization, or numerical stability. The solve was treated as routine linear algebra.

#### Root Cause

The **discretised Fredholm integral equation of the second kind** produces an operator matrix A whose eigenvalues typically span many orders of magnitude — 10⁶ or wider is common for bispectral kernel problems. This extreme ill-conditioning arises from the nature of the bispectrum-to-Volterra relationship: the kernel H₂(ω₁, ω₂) at frequencies where the bispectrum is small (near the principal domain boundaries, or at frequencies the sensor barely excites) must be extrapolated from very weak signal, and the resulting matrix entries make the linear system nearly singular.

`scipy.linalg.solve` (which calls LAPACK's `dgesv` — LU factorisation, no regularization) will:
1. Attempt to solve the system exactly
2. Amplify numerical noise in the near-singular directions by a factor equal to the condition number `κ(A) = σ_max / σ_min`
3. Return a solution vector where some coefficients have been inflated by `κ(A)` × the floating-point rounding error

For a typical bispectral Fredholm system, `κ(A)` can be 10⁶–10⁸. This means rounding errors at the 10⁻¹⁶ level (float64 machine epsilon) blow up to 10⁻¸ – 10⁻⁸ errors in the output coefficients. The resulting Volterra kernel h₂ will contain large spurious oscillations. When this kernel is applied as a convolution filter, it will introduce severe ringing and gain errors into the corrected signal.

#### Consequence if Left Unresolved

- The inverse Volterra filter computes `x_volt = Σ h₁[τ]·x[n-τ] + ΣΣ h₂[τ₁,τ₂]·x[n-τ₁]·x[n-τ₂]`. Corrupted h₂ coefficients produce a large quadratic error injection into every output sample
- The corrected signal `x_volt` may be *more* distorted than the raw uncalibrated sensor output
- The effect is sensor-dependent and hard to predict: it may only appear under certain excitation conditions, making it difficult to catch without careful validation
- A demo with this bug would show SPECTER's corrected output diverging or oscillating under the stress test, destroying the demo narrative at the worst possible moment
- The failure mode is subtle enough that it might not be caught during casual bench testing, only surfacing under dynamic excitation

#### Fix Applied

Replaced `scipy.linalg.solve` with **Tikhonov regularization** (ridge regression) on the Fredholm system. The regularized solve is:

```
h2 = np.linalg.lstsq(A + lambda_reg * np.eye(A.shape[0]), b, rcond=None)[0]
```

Where `lambda_reg = 0.01 × ||A||_F` — 1% of the Frobenius norm of A, computed as `np.linalg.norm(A, 'fro')`. This regularization parameter:

- **Adds a small positive diagonal perturbation** to A, lifting all near-zero singular values by at least `lambda_reg`. The modified condition number becomes `κ_reg = σ_max / (σ_min + λ_reg)`, which is bounded above by `||A||_F / λ_reg = 100` — regardless of how ill-conditioned the original A was.
- **Is scale-adaptive**: `lambda_reg` is proportional to `||A||_F`, so it scales correctly across different sensors with different bispectral magnitudes. No manual tuning needed.
- **Is re-estimated each bootstrap** from the current bispectrum, so it automatically adjusts to different sensor types and SNR conditions when a new sensor is plugged in.
- **Introduces a small, bounded bias**: the solution trades an infinitesimal bias increase for eliminating coefficient magnitudes of O(`κ(A) × ε_machine`). In practice, for bispectrum-extracted kernels, this tradeoff is heavily in favour of regularization — the numerical noise removed is orders of magnitude larger than the physical information lost.

The `np.linalg.lstsq` call is robust to any remaining ill-conditioning via its internal SVD, providing a further safety layer beyond the diagonal perturbation.

#### Sections Updated in PRD

- Section 7.1 Step 7 — Full explanation of ill-conditioning, `lambda_reg = 0.01 × ||A||_F` derivation, one-line `np.linalg.lstsq` implementation
- Section 10 — New risk row added: "Fredholm operator ill-conditioning (Medium)" with Tikhonov mitigation and adaptive re-estimation note

---

### ISSUE-004 — HC-SR04 Ultrasonic Sensor Incompatible with Bispectral Bootstrap

| Field | Detail |
|---|---|
| **Date** | March 13, 2026 |
| **Time** | Session 3 |
| **Severity** | Medium — causes silent bootstrap failure on the plug-in demo sensor; the bispectrum algorithm produces a meaningless result with no error message |
| **Status** | ✅ Resolved |

#### Problem Description

Section 5.5 of the original PRD listed the third (plug-in demo) sensor as:

> *"Any commodity sensor — a photoresistor, a cheap NTC thermistor, an HC-SR04 ultrasonic sensor — for the live plug-in demonstration of SPECTER's sensor-agnostic bootstrap."*

The HC-SR04 was listed as a valid option for the Wow Factor 3 live plug-in demo — where an unknown sensor is plugged in mid-presentation and SPECTER bootstraps it cold in under 10 seconds.

#### Root Cause

The entire bispectral identification pipeline rests on a foundational statistical assumption stated in Section 4.1:

> *"The bispectrum of the sensor output converges to a function determined entirely by the sensor's nonlinear transfer function — regardless of what the input signal was, as long as the input is **approximately Gaussian or sub-Gaussian**. The input statistics wash out during the averaging process, leaving only the sensor's fingerprint."*

This washout property only holds when the input signal is **continuous, broadband, and approximately Gaussian**. It fails completely for:

1. **Discrete-valued outputs**: The HC-SR04 measures time-of-flight using a 40 kHz ultrasonic pulse and returns a single integer distance value per measurement cycle (typically at 20–40 Hz). There is no continuous analog output. The sensor does not produce a signal that can be windowed into 512-sample frames and FFT'd meaningfully.
2. **Periodic or sparse excitation**: The HC-SR04's output between measurements is zero (no signal). A 512-sample Hanning-windowed frame of HC-SR04 output is almost entirely zeros with sparse non-zero transitions. The FFT of this is a sinc-like function with no broadband structure. The bispectrum of this signal encodes the measurement pattern, not the sensor's nonlinear transfer function.
3. **No analog signal path**: SPECTER's Volterra model assumes a sensor with a continuous signal path: `y(t) = H₂[f(H₁[x(t)])] + n(t)`. The HC-SR04 is a digital device — its output is a GPIO pulse width, not an analog voltage proportional to a physical quantity. There is nothing for the Wiener-Hammerstein model to identify.

The consequence is that the bispectrum estimator would run on the HC-SR04 output and produce a structured-looking but completely meaningless bispectrum. The Fredholm solve would extract nonsense kernels. The Volterra inverse filter would apply random distortion to the already-meaningless signal. SPECTER would appear to bootstrap successfully (confidence score might still reach a nominal value) while producing calibrated output that is pure artifact.

#### Consequence if Left Unresolved

- The Wow Factor 3 plug-in demo — the single most memorable demo moment — would silently produce garbage output
- Corrected output would be meaningless but visually indistinguishable from a real calibration result to non-expert judges
- Any judge who happened to look at the bispectrum heatmap would see an unusual, non-physical pattern and flag it as suspicious
- If a technically expert judge asked "what is the sensor's frequency response?", the forensics report would return nonsensical values that cannot be physically explained
- The claim that SPECTER is "sensor-agnostic" would technically be false, since it fails on a large class of sensors

#### Fix Applied

Removed the HC-SR04 from the permitted sensor list entirely. Section 5.5 now explicitly states:

> *"The bootstrap requires a signal with a continuous-spectrum sub-Gaussian distribution; the bispectrum estimator exploits this property to wash out input statistics and expose the sensor’s fingerprint. Sensors with discrete, quantised, or periodic output (e.g. HC-SR04 ultrasonic rangefinder, I2C digital sensors, PWM outputs) are incompatible and will produce unreliable bispectrum estimates."*

The recommended plug-in demo sensor is now the **NTC thermistor**, which is ideal for this application:
- Produces a continuous analog voltage output proportional to temperature
- Its Johnson (thermal) noise provides broadband Gaussian excitation naturally, with no external signal source required
- The nonlinear thermistor equation `R(T) = R₀ · exp(B·(1/T - 1/T₀))` is a classic Volterra-identifiable nonlinearity — exponential, memoryless static nonlinearity sandwiched between the thermal lag (H₁) and the measurement circuit (H₂)
- It costs ~₹10 and is universally available
- Its bispectrum will look visually different from the voltage divider module's bispectrum — providing a clear, compelling visual payoff on the dashboard during the plug-in demo

#### Sections Updated in PRD

- Section 5.5 (Sensors table) — HC-SR04 removed; NTC thermistor specified as recommended demo sensor; explicit incompatibility statement added for discrete/digital output sensors

---

### ISSUE-005 — asyncio Single-Threaded Event Loop Cannot Parallelise CPU-Bound DSP Workers

| Field | Detail |
|---|---|
| **Date** | March 13, 2026 |
| **Time** | Session 3 |
| **Severity** | High — under load, CPU-bound EKF matrix operations will starve the compensation hot path, causing latency spikes and violating the “under 15 ms end-to-end” claim |
| **Status** | ✅ Resolved |

#### Problem Description

The original PRD specified the Raspberry Pi 5 software stack as:

> *"Python 3.11 with NumPy, SciPy, asyncio for the web server. All DSP runs in NumPy vectorised operations — no pure Python loops in the hot path."*

Section 8.1 described all modules (`acquire.py`, `bootstrap.py`, `compensate.py`, `kalman.py`, etc.) as tasks within a single `asyncio` event loop orchestrated by `main.py`.

#### Root Cause

`asyncio` is a **single-threaded cooperative multitasking** framework. It is designed for I/O-bound concurrency — situations where coroutines spend most of their time waiting on network sockets, file handles, or serial ports. The scheduler interleaves coroutines by `await`ing I/O operations; while one coroutine waits, another runs.

CPU-bound NumPy operations — Volterra convolution, EKF matrix inversion, bispectrum accumulation — **do not `await` anything**. They run straight through on the CPU until completion. An asyncio coroutine that calls `scipy.linalg.solve` on an 80×80 matrix will hold the event loop for the entire duration of the solve (100–300 μs). No other coroutine can run during this time.

The result is priority inversion: the compensation hot path (which must run every 50–100 μs for the Wiener equalizer) contends with the EKF (which runs at 100 Hz = every 10 ms). When the EKF solve runs, the compensation hot path is blocked. Under peak load — when bootstrap, EKF, compensation, and telemetry are all active simultaneously — end-to-end latency will spike unpredictably.

Additionally, Python's **Global Interpreter Lock (GIL)** prevents any two Python threads from executing Python bytecode simultaneously. Even if `asyncio` were replaced with `threading`, NumPy calls would release the GIL internally — but the Python level synchronisation around shared data structures (ring buffers, coefficient arrays) would still create serialisation points under contention.

The Pi 5 has **four Cortex-A76 cores**. Under the original asyncio architecture, all DSP work serialises onto a single core. Three cores sit idle. The "under 15 ms end-to-end" latency guarantee fails because it assumes work is distributed, but it is not.

#### Consequence if Left Unresolved

- The compensation hot path latency is non-deterministic under load. During the bootstrap phase (CPU-intensive) or during adversarial stress test demos (EKF + CUSUM running hard), corrected output samples would be delayed or dropped
- The CUSUM detector runs in the same loop as compensation; if the EKF holds the loop for 300 μs, CUSUM doesn't fire on time, and the autonomous re-bootstrap trigger is delayed
- The Pi 5's quad-core hardware advantage is entirely wasted — a Pi 3 would perform identically under this architecture
- Latency spikes would be visible as glitches in the live dashboard waveforms, undermining judge confidence in the system's real-time capability

#### Fix Applied

Replaced the single asyncio event loop architecture with a **four-process multiprocessing design** that dedicates one OS process to each Pi 5 core. Each process is CPU-affinity-pinned using `os.sched_setaffinity`, ensuring true parallel execution with zero GIL contention:

| Process | Core | Modules | Concurrency Type |
|---|---|---|---|
| Acquisition | Core 0 | `acquire.py` | asyncio (correct here — I/O-bound USB serial) |
| Compensation | Core 1 | `compensate.py` | Tight NumPy loop, no scheduler |
| Tracking | Core 2 | `kalman.py`, `crossmodal.py` | 100 Hz CPU-bound EKF |
| Services | Core 3 | `bootstrap.py`, `forensics.py`, `telemetry.py` | Triggered tasks + 10 Hz telemetry |

**Coefficient handoff between processes** uses `multiprocessing.shared_memory` — a named POSIX shared memory region mapped into both the Tracking (writer) and Compensation (reader) address spaces. The coefficient vector is written atomically by the EKF process at 100 Hz; the compensation process reads it at every sample without any locking, blocking, or IPC call. Zero-copy. Zero synchronisation latency in the hot path.

**Why `shared_memory` and not a Queue or Pipe**: `multiprocessing.Queue` and `Pipe` involve serialisation (pickle), OS-level context switches, and bounded buffer management. For a coefficient vector that is updated 100 times per second and read at potentially 10,000 times per second, this overhead is unacceptable. `shared_memory` gives direct pointer-level access with no serialisation and no blocking.

**Bootstrap process isolation**: The bootstrap process (Core 3) takes up to 8 seconds of heavy CPU work. Under the old architecture, this would completely block the event loop and suspend all compensation. Under the new architecture, it runs on a dedicated core while Core 1 continues applying last-known-good coefficients to every incoming sample throughout the entire bootstrap duration. The coefficient swap on bootstrap completion is a single pointer update in shared memory — effectively atomic and imperceptible.

#### Sections Updated in PRD

- Section 5.3 (Pi 5 software stack) — "asyncio" replaced with "multi-process architecture using `multiprocessing`"; `shared_memory` coefficient handoff described
- Section 7.3 (Phase 3 EKF drift tracking) — "background asyncio task" replaced with "dedicated background process pinned to Core 2"
- Section 8.1 (Raspberry Pi 5 Python Pipeline) — Entire section rewritten: four-process layout table with Core assignments, module-to-process mapping, `shared_memory` mechanism, `os.sched_setaffinity` CPU pinning, and health monitoring via heartbeat flags explained in detail

---

## Investigated — Not a Real Issue

---

### INVESTIGATE-001 — Claim: 80-Dimensional EKF at 100 Hz Exceeds Pi 5 Compute Budget

| Field | Detail |
|---|---|
| **Date** | March 13, 2026 |
| **Verdict** | ❌ Dismissed — arithmetic error in the challenge; PRD numbers are correct |

#### Claim Made

> *"The EKF covariance update involves inverting an (80×80) matrix. That’s O(n³) = 512,000 floating-point operations per update, at 100 Hz. NumPy’s scipy.linalg.solve is good, but on the Raspberry Pi 5 ARM Cortex-A76, this is ~51 million matrix operations per second — which is at the edge of real-time budget."*

#### Why This Is Wrong

The challenger's calculation is: `80³ × 100 Hz = 51.2 × 10⁶ FLOPS/s = 51 MFLOPS/s` and concludes this is "at the edge of real-time budget."

The error is in the denominator. The Raspberry Pi 5's Cortex-A76 cores with OpenBLAS-backed NumPy/SciPy sustain approximately **20–40 GFLOPS/s** (20,000–40,000 MFLOPS/s) on dense matrix operations. The EKF workload of 51 MFLOPS/s represents **0.1–0.25% of peak throughput** — a rounding error in the hardware's compute budget.

In practice, `scipy.linalg.solve` on an 80×80 float64 matrix on the Pi 5 executes in approximately **100–300 microseconds**. At 100 Hz (10 ms budget), this is **1–3% of the available time window**. The PRD claim of "under 2 ms for the full EKF update" is accurate and well within real-time budget even with Jacobian computation, covariance propagation, and CUSUM accumulation included.

**No PRD change was required.** The 80-dimensional EKF at 100 Hz is computationally trivial for this hardware.

---

## Acknowledged Risks (Valid Concerns, No PRD Change Required)

---

### RISK-001 — Bispectrum Convergence Frame Count May Be Insufficient

| Field | Detail |
|---|---|
| **Date** | March 13, 2026 |
| **Status** | ⚠️ Acknowledged — empirical validation required during implementation |

#### Concern

The PRD specifies 100 frames × 512 samples for bispectrum estimation. Literature on bispectrum-based system identification recommends significantly larger frame counts for reliable convergence, especially for non-stationary signals with limited ergodicity guarantees.

#### Assessment

This is a **real, valid concern** that cannot be resolved by PRD changes alone. The correct number of frames is sensor-dependent and must be determined empirically.

#### Existing Mitigation in PRD

Section 10 (Risk Table) already lists: *"Increase averaging window to M=200 frames. Accept slightly less accurate initial calibration and let the EKF refine it."* This is the right mitigation. The EKF will also continuously refine the kernel estimates post-bootstrap, compensating for initial bispectrum noise.

#### Action Required at Implementation

During firmware validation (Hours 9–14 in the build timeline), empirically test bispectrum stability: plot the bispectrum estimate as a function of frame count at M = 50, 100, 150, 200, and 300. Identify the frame count at which bispectral peaks stabilise to within 5% of their asymptotic value for each specific sensor. Use that as the operational M. If M > 200 is required, accept the longer bootstrap time and update the PRD estimate accordingly. The `±0.6–1.2%` headline error figure is aspirational until validated by this experiment.

---

## Summary Table

| Issue ID | Title | Date | Severity | Status |
|---|---|---|---|---|
| ISSUE-001 | Physically unjustifiable cross-modal sensor pair (accelerometer + pressure) | March 13, 2026 | High | ✅ Resolved |
| ISSUE-002 | Constant Modulus Algorithm applied to non-constant-modulus sensor signals | March 13, 2026 | Critical | ✅ Resolved |
| ISSUE-003 | Fredholm kernel extraction numerically unstable without regularization | March 13, 2026 | High | ✅ Resolved |
| ISSUE-004 | HC-SR04 ultrasonic sensor incompatible with bispectral bootstrap | March 13, 2026 | Medium | ✅ Resolved |
| ISSUE-005 | asyncio single-threaded event loop cannot parallelise CPU-bound DSP workers | March 13, 2026 | High | ✅ Resolved |
| INVESTIGATE-001 | Claim that 80-dim EKF at 100 Hz exceeds Pi 5 compute budget | March 13, 2026 | — | ❌ Dismissed (arithmetic error in claim) |
| RISK-001 | Bispectrum convergence may require more than 100 frames | March 13, 2026 | Medium | ⚠️ Acknowledged (empirical validation needed) |

---

*SPECTER Issues Log · Last updated March 13, 2026*
