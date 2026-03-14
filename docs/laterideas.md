# SPECTER - Later Ideas (Post-Baseline Roadmap)

**Purpose:** Capture the advanced architecture ideas discussed for the post-baseline version of SPECTER, so implementation can begin later without losing technical decisions.

**Status:** Future work (implement only after current Python baseline is fully working and measured on real hardware).

**Date Recorded:** March 13, 2026

---

## 1) Why This Exists

The current plan is correct:
- First, build and validate the existing architecture end-to-end.
- Then, evolve to a stronger resume-grade architecture with embedded C++ compute on Teensy.

This file documents the second stage in detail.

---

## 2) Strategic Direction

### 2.1 Current Baseline (Build First)

- Teensy 4.1: deterministic sensor acquisition + framing + USB stream
- Raspberry Pi 5: bispectrum, Volterra inversion, Wiener equalizer taps, EKF/CUSUM, telemetry
- ESP32-S3: dashboard

### 2.2 Later Upgrade (This Document)

Use a **hybrid split architecture**:
- Keep heavy bootstrap identification on Pi 5
- Move hard real-time compensation hot path to Teensy in C++

This gives:
- deterministic microsecond-level correction latency
- stronger embedded signal-processing credibility
- cleaner resume value for firmware/DSP roles

---

## 3) Target Hybrid Architecture (Detailed)

## 3.1 Core Principle

- **Pi 5 computes coefficients** (slow, heavy, matrix-heavy, memory-heavy tasks).
- **Teensy applies coefficients** at sample rate (fast, deterministic, real-time tasks).

## 3.2 Hardware Roles

### Teensy 4.1 (C++ / CMSIS-DSP / optional FreeRTOS)

Primary responsibilities:
- Sensor acquisition (voltage + INA219 current)
- Timestamping and sample integrity
- Real-time compensation using latest coefficients
- Lightweight runtime health metrics
- USB protocol endpoint for coefficient updates and telemetry uplink

### Raspberry Pi 5 (Python + NumPy/SciPy)

Primary responsibilities:
- Bootstrap bispectrum estimation
- Tikhonov-regularized Fredholm kernel solve
- Wiener tap synthesis
- Optional supervisory EKF/CUSUM or parameter estimation pipeline
- Forensics report generation
- Dashboard telemetry aggregation

### ESP32-S3

Primary responsibilities:
- Dashboard serving only
- visual status and mobile access

---

## 4) Compute Partition (What Runs Where)

## 4.1 Teensy - Real-Time Path

Runs continuously per sample/frame:
- FIR Wiener equalizer (64 taps)
- Volterra compensation (truncated second-order terms selected for budget)
- Cross-modal error calculation: `epsilon = ||V_corrected - I_corrected * R_load||^2`

Design requirement:
- No dynamic allocation in hot path
- Pre-allocated buffers only
- Double-buffered coefficient swap

## 4.2 Pi 5 - Bootstrap and Heavy Solve

Runs on startup and re-bootstrap events:
- accumulate M frames for bispectrum
- estimate H1 and H2
- solve Fredholm with Tikhonov regularization
- compute Wiener frequency response and IFFT to taps
- package updated coefficient set and send to Teensy

---

## 5) Data Flow and Interfaces

## 5.1 Streaming Direction

Runtime stream:
- Teensy -> Pi: corrected samples, raw samples (optional debug mode), epsilon, timestamps, health counters

Control stream:
- Pi -> Teensy: coefficient update packets, mode switches, sync/reset commands

## 5.2 Coefficient Update Packet (Proposed)

Binary packet layout (example):
- `magic` (2 bytes)
- `version` (1 byte)
- `msg_type` (1 byte)  # COEFF_UPDATE
- `seq_id` (4 bytes)
- `tap_count` (2 bytes)
- `kernel_count` (2 bytes)
- `payload_len` (4 bytes)
- `payload` (float32 array)
- `crc32` (4 bytes)

Payload order:
1. Wiener taps (N=64)
2. Linear Volterra terms
3. Quadratic dominant terms
4. Optional EKF state subset

Acceptance rule on Teensy:
- Validate magic/version/type
- Validate length and CRC
- Write to inactive coefficient buffer
- Atomic pointer swap at frame boundary
- ACK with `seq_id`

---

## 6) Real-Time Scheduling Plan (Teensy)

Two valid options:

## 6.1 Option A - Interrupt + Main Loop (simpler)

- Timer ISR handles sampling trigger
- DMA completion interrupt marks frame ready
- Main loop executes compensation on ready frame
- USB TX in low-priority section

Use this first unless complexity demands RTOS.

## 6.2 Option B - FreeRTOS Tasks (scalable)

Task split (example):
- Task 1 `acquire_task` (highest)
- Task 2 `dsp_task` (high)
- Task 3 `comms_task` (medium)
- Task 4 `diag_task` (low)

Only move to this after Option A is stable.

---

## 7) Numeric Stability Requirements

## 7.1 Fredholm Solve (Pi side)

Always use regularized solve:
- `h2 = lstsq(A + lambda_reg * I, b)`
- `lambda_reg` adaptive to operator norm or cross-validated over held-out frames

## 7.2 Coefficient Quantization

- Compute in float64 on Pi where needed
- transmit as float32 to Teensy
- validate numerical drift versus float64 reference

## 7.3 Saturation and Guard Rails on Teensy

- Clamp outlier coefficients at load time
- check NaN/Inf before swap
- watchdog rollback to last-known-good coefficients on anomaly

---

## 8) Latency and Throughput Targets

## 8.1 End-to-End Targets

- compensation compute per frame: deterministic and bounded
- no dropped sample frames under normal thermal load
- coefficient swap glitch-free (no discontinuity spike)

## 8.2 Measurement Plan

Instrument:
- ISR entry/exit timestamps
- DSP block runtime histogram (min/avg/p95/p99/max)
- USB queue depth and overflow counters
- coefficient update apply latency

Success condition:
- bounded p99 latency with zero instability during stress tests

---

## 9) Validation Strategy (Critical)

## 9.1 Gold Reference

Keep Python baseline as truth model.

For each algorithm block:
- run identical input windows through Python and Teensy implementations
- compare outputs with tolerance thresholds
- store error envelopes in test artifacts

## 9.2 Test Ladder

1. Unit tests on synthetic signals
2. Hardware-in-loop with recorded buffers
3. Live sensor runs under thermal stress
4. re-bootstrap stress with forced coefficient changes

## 9.3 Required Test Artifacts

- overlay plots (Python vs Teensy outputs)
- residual distributions
- worst-case latency report
- coefficient swap continuity plot

---

## 10) Migration Plan (Practical Sequence)

## Phase 0 - Baseline First (must finish)

- finish current architecture
- verify all claims with measured metrics
- freeze a stable baseline tag

## Phase 1 - Teensy DSP Skeleton

- add fixed-point/float buffer infrastructure
- add FIR path only
- verify against Python FIR outputs

## Phase 2 - Add Volterra Path

- add linear + selected quadratic terms
- profile runtime and memory
- compare to Python block output

## Phase 3 - Coefficient Protocol

- implement packet parser + CRC + ACK
- double-buffer coefficient swap
- integrate Pi sender utility

## Phase 4 - Runtime Robustness

- watchdog + rollback
- telemetry counters
- stress and endurance tests

## Phase 5 - Optional EKF Placement Decision

Evaluate two alternatives:
- EKF remains on Pi (recommended initial hybrid)
- EKF moved to Teensy only if profiling proves comfortable margin and code quality remains high

---

## 11) Risks and Mitigations

## 11.1 Risk: Overloading Teensy with full matrix-heavy pipeline

Mitigation:
- keep bootstrap solve on Pi permanently
- move only deterministic hot path to Teensy

## 11.2 Risk: Coefficient corruption over serial

Mitigation:
- CRC32 + sequence ID + ACK/NACK + timeout retransmit

## 11.3 Risk: Runtime instability after coefficient update

Mitigation:
- stage new coefficients in inactive buffer
- validate finite range
- swap only at frame boundary
- fallback to last-known-good set

## 11.4 Risk: Project scope explosion

Mitigation:
- enforce migration phases
- no phase skipping
- baseline must remain runnable at all times

---

## 12) Resume Positioning (Future Version)

Once implemented cleanly, the project can be described as:

- Designed hybrid DSP architecture across Cortex-M7 + Linux SBC
- Implemented real-time Wiener + Volterra compensation pipeline in C++ on Teensy
- Built robust coefficient hot-swap protocol with atomic double buffering and CRC
- Used bispectral blind identification + Tikhonov regularized inverse solve on Pi
- Achieved deterministic correction pipeline with measured latency distributions under stress

This framing is strong for embedded firmware, controls, robotics, and applied DSP interviews.

---

## 13) Decision Log

- Decision: do **not** start this migration until baseline is fully working.
- Decision: keep Python baseline as permanent reference implementation.
- Decision: prioritize deterministic hot path on Teensy before any additional features.

---

## 14) What To Do When Reopening This Plan

When you return to this file later:

1. Confirm baseline tag exists and is stable.
2. Start at Phase 1 only.
3. Keep a benchmark table per phase.
4. Do not move EKF to Teensy until FIR + Volterra + protocol are stable.
5. Update this file with measured numbers, not assumptions.

---

*Document owner: SPECTER roadmap notes*
*Path: `docs/laterideas.md`*
