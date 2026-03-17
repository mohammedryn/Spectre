# commands.md — Pi and Teensy Command Reference

## Purpose

This document records the exact command flow used to build, upload, monitor, and troubleshoot the Teensy 4.1 firmware from the Raspberry Pi for the SPECTER project.

The goal is to keep a practical operator reference inside the repository so the hardware validation workflow is reproducible without re-deriving the shell commands each time.

## Current Context

At this stage of the project, Chunk 1 and the code implementation parts of Chunk 2 are complete.

The current validated environment is:

- Raspberry Pi 5 running Ubuntu 24.04
- Project cloned at `~/Spectre`
- Python environment created with Python 3.11 via `uv`
- Python dependencies installed successfully inside `~/Spectre/.venv`
- Teensy 4.1 connected to the Pi over USB
- INA219 connected over I2C to Teensy and detected at runtime

One important environment detail is that PlatformIO upload to Teensy currently works reliably only when run with `sudo` and the full path to the virtual environment Python executable.

Builds may fail as a normal user if disk space, ownership, or PlatformIO cache state is inconsistent. Uploads may fail as a normal user because the Teensy bootloader device requires elevated USB access on the current Pi setup.

## Environment Activation

Use these commands after logging into the Pi.

```bash
cd ~/Spectre
source .venv/bin/activate
python --version
```

Expected result:

- `python --version` should report Python 3.11.x

Why this matters:

- The project requirements target Python 3.11
- The pinned `scipy==1.12.0` dependency failed under Python 3.13 on the Pi
- The Python 3.11 environment avoids building incompatible dependency versions from source

## Dependency Installation on Pi

If the environment must be recreated, use the following sequence.

### Install `uv`

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
source ~/.local/bin/env
```

### Install Python 3.11 and create virtual environment

```bash
uv python install 3.11
cd ~/Spectre
uv venv --python 3.11 .venv
source .venv/bin/activate
python --version
```

### Install Python dependencies

```bash
uv pip install --upgrade pip setuptools wheel
uv pip install -r requirements.txt
python -m pytest tests/ --collect-only
```

Why this matters:

- This is the confirmed-good path on the Pi
- `pytest --collect-only` verifies the environment is usable before touching hardware

## PlatformIO Invocation

PlatformIO may not be available as a direct `pio` command inside the virtual environment until it is installed explicitly.

Install it with:

```bash
python -m pip install -U platformio
python -m platformio --version
```

Recommended usage in this repository:

```bash
python -m platformio <subcommand>
```

Why this matters:

- It avoids depending on shell aliases
- It ensures the PlatformIO instance comes from the active project environment

## Teensy Firmware Build

Build from the Teensy firmware directory.

```bash
cd ~/Spectre/firmware/teensy
python -m platformio run
```

Expected result:

- PlatformIO completes with `SUCCESS`
- Firmware artifacts are generated under `.pio/build/teensy41/`

Why this matters:

- This validates the firmware source tree and toolchain
- It produces the `.hex` file needed for upload

## Teensy Firmware Upload

### Recommended upload command on the Pi

```bash
cd ~/Spectre/firmware/teensy
sudo "$HOME/Spectre/.venv/bin/python" -m platformio run -t upload
```

Why `sudo` is currently required:

- Teensy upload uses low-level bootloader USB access
- On the current Pi setup, normal user permissions are not consistently sufficient for bootloader access
- Using the virtual environment Python path ensures `sudo` still runs the correct PlatformIO installation

Expected upload pattern:

```text
Rebooting...
Uploading .pio/build/teensy41/firmware.hex
Teensy Loader, Command Line, Version 2.2
Found HalfKay Bootloader
Programming.......................
Booting
```

Expected final result:

- PlatformIO exits with `SUCCESS`

## Direct Teensy Loader Upload

If PlatformIO upload flow needs to be bypassed and the `.hex` file is already built, use the loader directly.

```bash
cd ~/Spectre/firmware/teensy
sudo ~/.platformio/packages/tool-teensy/teensy_loader_cli -mmcu=imxrt1062 -w -v .pio/build/teensy41/firmware.hex
```

Why this is useful:

- It bypasses the full PlatformIO upload wrapper
- It is useful when the board is already in HalfKay bootloader mode

## Serial Monitor

Open the Teensy serial monitor from the Pi with:

```bash
cd ~/Spectre/firmware/teensy
python -m platformio device monitor -b 115200 -p /dev/ttyACM0
```

Expected runtime output for Chunk 2:

```text
INA219 OK
V_RAW=...
I_RAW=...
```

Important note:

- If the monitor is opened after boot, the startup line may be missed
- To capture startup status, keep the monitor open and press the Teensy reset button once

## USB State Checks

These commands were used repeatedly to determine whether Teensy was in bootloader mode or runtime serial mode.

### Check USB identity

```bash
lsusb | grep -i 16c0
```

Interpretation:

- `16c0:0478` means Teensy is in HalfKay bootloader mode
- `16c0:0483` means Teensy is running normal serial firmware mode

### Check serial node

```bash
ls /dev/ttyACM* 2>/dev/null || echo "no ttyACM"
```

Interpretation:

- `/dev/ttyACM0` present means runtime serial mode is active
- no `ttyACM` is normal while the board is in bootloader mode

## Chunk 2 Runtime Validation Context

The following behaviors were observed and are important for interpreting results.

### Case 1: Nothing connected to Teensy inputs

Observed behavior:

- `V_RAW` changed continuously
- `I_RAW` remained `0`

Meaning:

- `V_RAW` was floating because analog pin `A0` had no connected signal
- `I_RAW` remained zero because INA219 was not yet wired or no current was flowing

This does not satisfy full Chunk 2 hardware validation.

### Case 2: INA219 connected and detected

Observed behavior:

```text
INA219 OK
V_RAW=814 I_RAW=0
V_RAW=279 I_RAW=0
...
```

Meaning:

- I2C communication with INA219 is working
- The firmware setup path for Chunk 2 is correct
- Current is still zero because no meaningful current is flowing through the INA219 shunt path yet

To fully satisfy Chunk 2 manual validation, current must actually pass through the shunt and `I_RAW` must respond to load changes.

## INA219 Wiring Reference

For the specific blue INA219 breakout used during validation, the confirmed logic-side wiring is:

- `Vcc` -> Teensy `3.3V`
- `Gnd` -> Teensy `GND`
- `Sda` -> Teensy pin `18`
- `Scl` -> Teensy pin `19`

Current path wiring:

- source positive -> `Vin+`
- `Vin-` -> load positive
- source negative and load negative remain common ground

Why this matters:

- INA219 measures current only when real current flows through the shunt path between `Vin+` and `Vin-`
- Merely powering the module is not enough to produce a non-zero current reading

## Common Failure Modes and Meaning

### `INA219 FAIL`

Meaning:

- INA219 configuration write/readback failed
- Usual causes are wiring errors, lack of power, swapped SDA/SCL, or incorrect address assumptions

### `Error opening USB device: Resource temporarily unavailable`

Meaning:

- Teensy bootloader device was detected but could not be opened
- On the Pi this was worked around by using `sudo` for upload

### `Disconnected ... Reconnecting to /dev/ttyACM0`

Meaning:

- USB serial reset occurred, usually because the board rebooted or the monitor saw a transient disconnect
- This is expected around reset/upload cycles and is not by itself a firmware bug

### `No space left on device`

Meaning:

- The Pi ran out of storage during build or SCons metadata writeback

Recovery commands:

```bash
python -m platformio system prune -f
cd ~/Spectre/firmware/teensy
rm -rf .pio
pip cache purge
sudo apt clean
sudo apt autoremove -y
sudo journalctl --vacuum-time=3d
df -h
```

## Current Recommended Workflow

This is the shortest stable workflow confirmed so far.

### Build

```bash
cd ~/Spectre/firmware/teensy
source ~/Spectre/.venv/bin/activate
python -m platformio run
```

### Upload

```bash
sudo "$HOME/Spectre/.venv/bin/python" -m platformio run -t upload
```

### Monitor

```bash
python -m platformio device monitor -b 115200 -p /dev/ttyACM0
```

### Capture startup line

With the monitor already open, press the Teensy reset button once.

## What Has Been Confirmed So Far

- Chunk 1 firmware scaffold compiles
- Chunk 2 code implementation compiles
- Upload from the Pi works
- Teensy runtime serial mode is reachable on `/dev/ttyACM0`
- INA219 can be detected successfully on I2C
- Runtime output cadence is correct at about one line every 500 ms

## What Still Must Be Verified for Full Chunk 2 Hardware Signoff

- A real analog input must be connected to `A0` so `V_RAW` represents an actual signal instead of a floating pin
- A real current path must be wired through INA219 so `I_RAW` changes with load
- The final monitor capture should show both voltage and current readings responding to physical variation