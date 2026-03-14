"""
SPECTER — Self-calibrating Pipeline for Estimating Characteristics
of Totally Unknown Embedded Sensors in Real-time.

Process layout:
  Core 0: acquire.py   — USB serial reader + ring buffer
  Core 1: compensate.py — Volterra + Wiener hot path
  Core 2: kalman.py    — 80-dim EKF + CUSUM
  Core 3: bootstrap.py / forensics.py / telemetry.py (services)
  Main:   main.py      — orchestrator + state machine
"""
__version__ = "0.1.0"
