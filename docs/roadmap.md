# SPECTER Full Learning Roadmap (Beginner to Solo Builder)

**Goal:** Teach you exactly what to learn, in what order, and how to practice, so you can build the complete SPECTER system from scratch by yourself.

**Who this is for:** Someone who currently feels like a beginner and wants a complete, practical, step-by-step path.

**End state:** You can independently implement, test, and explain the full SPECTER pipeline — from bare-metal Teensy firmware through graduate-level DSP to a live wireless dashboard — and defend every design decision in an interview.

---

## How To Use This Roadmap

Do not read this as theory only. Treat each phase as a build sprint with strict deliverables.

For every phase:
1. Learn the concepts.
2. Build the mini-project.
3. Pass the exit criteria.
4. Write notes in your own words.
5. Move only then.

If you skip the mini-projects, you will understand less than you think.

---

## Master Timeline

| Path | Duration |
|---|---|
| Aggressive full-time | 14–18 weeks |
| Minimum serious (12–18 hrs/week) | 20–28 weeks |
| Part-time student | 24–36 weeks |

---

# PHASE 1 — Computing and Tooling Foundations (Week 1–3)

Without this, you cannot move fast when firmware, serial, and matrix math break at once.

---

## 1.1 Linux Shell (Days 1–3)

**Navigation and file ops — learn every single one:**
- `cd`, `ls -la`, `pwd`, `mkdir -p`, `rm -rf`, `cp -r`, `mv`
- Hidden files and dotfiles (`.bashrc`, `.gitignore`)
- Absolute vs relative paths — know the difference cold
- `~` (home), `.` (current), `..` (parent) — use without thinking

**Text processing — you'll use these daily:**
- `cat`, `head -n`, `tail -n`, `tail -f` (watch live logs)
- `grep` — basic pattern matching, `-r` recursive, `-n` line numbers, `-i` case insensitive
- `grep -E` for extended regex (alternation `|`, groups `()`, quantifiers `+?*`)
- `ripgrep` (`rg`) — faster grep, use this in practice
- `sed` basics — `sed 's/old/new/g'` for find-replace in streams
- `awk` basics — `awk '{print $1, $3}'` for column extraction
- `wc -l` (line count), `sort`, `uniq`, `diff`

**Pipes, redirection, and composition:**
- `|` (pipe stdout to next command's stdin) — understand this is UNIX's core idea
- `>` (overwrite file), `>>` (append to file)
- `2>` (redirect stderr), `2>&1` (merge stderr into stdout)
- `<` (stdin from file)
- Command substitution: `$(command)` inside other commands
- `xargs` — convert stdin lines into arguments: `find . -name "*.py" | xargs grep "def"`

**Process management:**
- `ps aux`, `top`/`htop`, `kill`, `kill -9`
- `&` (background), `fg`, `bg`, `jobs`
- `Ctrl-C` (SIGINT), `Ctrl-Z` (SIGTSTP)
- `nohup` and `screen`/`tmux` basics — keep processes alive after logout

**File permissions:**
- `chmod` — understand octal (755, 644) and symbolic (u+x, g-w)
- `chown`, `chgrp`
- Why scripts need `chmod +x` before execution
- `ls -la` output: read the permission string `-rwxr-xr-x`

**Environment and shell config:**
- `$PATH` — what it is, how to add directories
- `export VAR=value`, accessing with `$VAR`
- `.bashrc` vs `.profile` — when each runs
- `which`, `whereis` — find where a binary lives
- `alias` — create shortcuts

---

## 1.2 Git Version Control (Days 3–5)

**Core model — understand this conceptually first:**
- Working directory → staging area → local repo → remote repo
- A commit is a snapshot of the entire project, not a diff
- HEAD, branches, and refs — what they point to
- `.git/` directory — know it exists, don't touch it

**Essential commands — memorize these:**
- `git init`, `git clone <url>`
- `git status` — run this constantly
- `git add <file>`, `git add -p` (stage hunks interactively — learn this, it's powerful)
- `git commit -m "message"`, `git commit --amend` (fix last commit)
- `git log --oneline --graph` — visual branch history
- `git diff` (unstaged changes), `git diff --staged` (staged changes)

**Branching — you will use this for every SPECTER feature:**
- `git branch <name>`, `git checkout -b <name>` (create + switch)
- `git switch <name>` (modern checkout)
- `git merge <branch>` — understand fast-forward vs 3-way merge
- `git rebase <branch>` — understand: replays your commits on top of target
- When to merge vs rebase — rebase for clean linear history, merge for collaborative branches
- `git rebase -i HEAD~3` — interactive rebase: squash, reword, reorder, drop commits

**Conflict resolution:**
- What a conflict looks like (`<<<<<<<`, `=======`, `>>>>>>>`)
- How to resolve: edit file, remove markers, `git add`, `git rebase --continue`
- `git merge --abort`, `git rebase --abort` — bail out safely

**Remote operations:**
- `git remote -v`, `git push`, `git pull`, `git fetch`
- `git push -u origin <branch>` — set upstream
- `git stash`, `git stash pop` — save and restore uncommitted work

**Commit message discipline:**
- Format: `type(scope): description` (e.g., `feat(ekf): implement prediction step`)
- Types: `feat`, `fix`, `refactor`, `docs`, `test`, `chore`

---

## 1.3 Python Foundations (Days 5–10)

**Data types and structures — know the runtime characteristics:**
- `int`, `float`, `str`, `bool`, `None`
- `list` — ordered, mutable, O(1) append, O(n) insert/delete
- `tuple` — ordered, immutable — use for fixed-size returns
- `dict` — hash map, O(1) average lookup, insertion order preserved (Python 3.7+)
- `set` — hash set, O(1) membership test, no duplicates
- List comprehensions: `[x**2 for x in range(10) if x % 2 == 0]`
- Dict comprehensions: `{k: v for k, v in pairs}`
- Generator expressions: `(x**2 for x in range(10))` — lazy, memory efficient

**Functions — beyond basics:**
- `def`, positional args, keyword args, `*args`, `**kwargs`
- Default mutable argument trap: `def f(x=[])` — know why this is a bug
- Type hints: `def process(data: np.ndarray, rate: float) -> np.ndarray:`
- Docstrings: write them for every function you build
- Lambda functions: `key=lambda x: x[1]` for sorting
- `functools.partial` — create specialized versions of functions

**Classes and OOP — you'll use this for the pipeline modules:**
- `class SensorPipeline:` with `__init__`, instance attributes, methods
- `self` — what it is (explicit reference to instance)
- `@property` — computed attributes without exposing internals
- `@staticmethod`, `@classmethod` — when to use each
- Inheritance basics — but prefer composition (has-a) over inheritance (is-a)
- `__repr__`, `__str__` — make your objects printable for debugging
- Dataclasses: `@dataclass` — use for config structs and parameter containers

**Error handling:**
- `try/except/finally` — catch specific exceptions, never bare `except:`
- `raise ValueError("message")` — raise meaningful errors
- Custom exceptions: `class BootstrapFailedError(Exception): pass`
- Context managers: `with open(...) as f:` — understand `__enter__`/`__exit__`

**Virtual environments — non-negotiable:**
- `python -m venv .venv`
- `source .venv/bin/activate` (Linux)
- `pip install -r requirements.txt`
- `pip freeze > requirements.txt`
- Why: isolation. Your Pi and dev machine must have identical deps.

**NumPy — this is your DSP engine:**
- Array creation: `np.zeros()`, `np.ones()`, `np.arange()`, `np.linspace()`
- Indexing: basic `a[2:5]`, boolean `a[a > 0]`, fancy `a[[1,3,5]]`
- Shape manipulation: `reshape()`, `squeeze()`, `expand_dims()`, `.T` (transpose)
- Broadcasting rules — learn the 3 rules, they dictate how operations combine differently-shaped arrays
- Element-wise ops: `+`, `*`, `**`, `np.sin()`, `np.exp()`, `np.abs()`
- Linear algebra: `np.dot()`, `@` operator, `np.linalg.solve()`, `np.linalg.inv()`, `np.linalg.lstsq()`
- `np.linalg.eig()`, `np.linalg.svd()` — you'll need these for condition number analysis
- `np.fft.fft()`, `np.fft.ifft()`, `np.fft.fftfreq()` — critical for DSP phases
- `np.conj()`, `np.real()`, `np.imag()` — complex number handling
- Performance: vectorize everything. If you write `for i in range(len(array)):` you're doing it wrong.
- `np.convolve()`, `np.correlate()` — convolution and correlation
- `np.random.normal()`, `np.random.seed()` — generating test signals

**Matplotlib — you must be able to plot fast:**
- `plt.plot()`, `plt.scatter()`, `plt.stem()`
- `plt.subplot()` / `fig, axes = plt.subplots(2, 2)`
- `plt.xlabel()`, `plt.ylabel()`, `plt.title()`, `plt.legend()`
- `plt.savefig()` — save plots for documentation
- `plt.imshow()` — for 2D bispectrum heatmaps
- `plt.semilogy()`, `plt.loglog()` — for frequency response plots

---

## 1.4 C/C++ Foundations (Days 10–16)

**Compilation model — understand what happens before runtime:**
- Preprocessor → Compiler → Assembler → Linker → Executable
- `#include` — textual copy-paste, nothing more
- `#define` macros — text substitution, no type safety
- `#pragma once` / include guards — prevent double inclusion
- Object files (`.o`), static libraries (`.a`), shared libraries (`.so`)
- `gcc -c file.c -o file.o` (compile), `gcc file.o -o program` (link)
- `g++ -std=c++17 -Wall -Wextra -O2` — always use warnings and optimization flags
- Makefiles: `target: dependencies \n\t command` — write a basic one by hand

**Data types and memory layout:**
- `int8_t`, `int16_t`, `int32_t`, `int64_t` — fixed-width types (`<stdint.h>`)
- `uint8_t`, `uint16_t` — unsigned variants (critical for register/protocol work)
- `float` (32-bit, ~7 digits), `double` (64-bit, ~15 digits) — know precision limits
- `sizeof()` — check sizes on your target platform
- Endianness: big-endian vs little-endian — ARM is little-endian, network is big-endian
- Struct packing and alignment: `struct __attribute__((packed))` — for binary protocols

**Pointers — you cannot do embedded without this:**
- `int *p = &x;` — p holds the address of x
- `*p` — dereference (read/write value at address)
- `NULL` / `nullptr` — null pointer, always check before dereferencing
- Pointer arithmetic: `p + 1` advances by `sizeof(*p)` bytes
- Arrays decay to pointers: `arr` and `&arr[0]` are the same
- `void*` — generic pointer, must cast before dereferencing
- Function pointers: `void (*callback)(int)` — used for ISR registration
- `const int *p` (pointer to const) vs `int * const p` (const pointer) — know the difference

**References (C++ only):**
- `int &ref = x;` — ref is an alias for x, not a copy
- Pass by reference: `void process(float &result)` — modifies original
- `const` references: `void read(const std::vector<float> &data)` — no copy, no modification

**Memory management:**
- Stack vs heap — stack is automatic, heap is manual
- `malloc()`/`free()` (C), `new`/`delete` (C++)
- Stack overflow — what causes it (deep recursion, large local arrays)
- Why embedded prefers stack allocation: no fragmentation, deterministic timing
- Buffer overflow — what it is, why it crashes

**Structs and enums:**
- `struct SampleFrame { uint32_t timestamp; int16_t voltage; int16_t current; uint8_t checksum; };`
- `enum class SystemState { COLD_BOOT, BOOTSTRAP, COMPENSATING, RE_BOOTSTRAP, SENSOR_SWAP };`
- Designated initializers (C): `struct Point p = {.x = 1, .y = 2};`

**Control flow and idioms:**
- `for`, `while`, `do-while`, `switch/case`
- Bit manipulation: `&` (AND), `|` (OR), `^` (XOR), `~` (NOT), `<<` (shift left), `>>` (shift right)
- Setting a bit: `reg |= (1 << bit_n)`
- Clearing a bit: `reg &= ~(1 << bit_n)`
- Checking a bit: `if (reg & (1 << bit_n))`
- These are used constantly in register-level embedded programming

---

## 1.5 Debugging Mindset (Days 16–18)

- The universal debugging loop: **Reproduce → Isolate → Measure → Fix → Verify**
- `print` debugging: useful but insufficient. Learn to add timing + values.
- Python: `pdb` / `breakpoint()` — step through code interactively
- C++: `gdb` basics — `break`, `run`, `next`, `step`, `print`, `backtrace`
- Reading error messages: start from the BOTTOM of the traceback
- Rubber duck debugging: explain the bug out loud before searching

---

## MINI-PROJECTS — Phase 1

**Project 1: CLI sensor log parser in Python**
- Read a CSV file of timestamped sensor values
- Compute moving average with window size N
- Compute standard deviation per window
- Plot raw vs smoothed with matplotlib, save to PNG
- Handle edge cases: missing values, non-numeric rows

**Project 2: N-tap FIR filter in C++**
- Implement `float fir_filter(float input, float *coeffs, float *delay_line, int num_taps)`
- Use a circular buffer for the delay line
- Generate coefficients in Python (e.g., low-pass via `scipy.signal.firwin`)
- Export coefficients as a C header array
- Process a test signal, export output, compare against NumPy `np.convolve()` reference
- They must match to within floating-point tolerance (~1e-6)

**Project 3: Git workflow drill**
- Create repo, make initial commit on `main`
- Create `feature/fir-filter` branch
- Make 3 commits with meaningful messages
- Create a deliberate conflict (edit same line on both branches)
- Resolve the conflict manually
- Interactive rebase: squash 3 commits into 1 with clean message

---

## EXIT CRITERIA — Phase 1

- [ ] Create and activate Python venv without looking at docs
- [ ] Compile and run a C++ program with `g++` from terminal
- [ ] Explain pointer vs reference in your own words (write it down)
- [ ] Write a 5-stage pipe command (e.g., `cat file | grep X | sort | uniq -c | head`)
- [ ] Use git rebase -i to squash commits without breaking anything
- [ ] Create a NumPy array, reshape it, broadcast-multiply it, FFT it
- [ ] Plot a signal and its FFT magnitude spectrum with labeled axes

---

# PHASE 2 — Electronics and Measurement Fundamentals (Week 4–6)

SPECTER fails if wiring, sampling, or power is unstable. Hardware confidence before algorithm confidence.

---

## 2.1 DC Circuit Theory (Days 1–3)

**Ohm's Law — the foundation of your cross-modal anchor:**
- `V = I × R` — know this in all three rearrangements
- Units: Volts [V], Amps [A], Ohms [Ω]
- What "resistance" physically means: opposition to current flow
- Power: `P = V × I = I²R = V²/R`
- Ohm's Law is the physics behind your cross-modal consistency check `ε = ||V_corrected − I_corrected × R_load||²`

**Kirchhoff's Laws:**
- KVL (Voltage Law): sum of voltages around any closed loop = 0
- KCL (Current Law): sum of currents entering a node = sum leaving
- Apply KVL to your voltage divider circuit: `V_in = V_R1 + V_R2`
- Apply KCL to verify current splits at junctions

**Voltage dividers — your voltage sensor IS one:**
- `V_out = V_in × R2 / (R1 + R2)`
- Loading effect: if you connect a load (like an ADC input), it changes the divider ratio
- Input impedance of the ADC matters — high impedance = minimal loading
- Temperature coefficient of resistors: resistors change value with temperature — this is the drift SPECTER corrects
- Calculate: for a 25V input with a 5:1 divider, what is V_out? What happens when R1 drifts +5%?

**Series and parallel combinations:**
- Series: `R_total = R1 + R2 + R3`
- Parallel: `1/R_total = 1/R1 + 1/R2` or `R_total = R1 × R2 / (R1 + R2)` for two resistors
- Capacitors in parallel add directly: `C_total = C1 + C2`
- Capacitors in series: `1/C_total = 1/C1 + 1/C2`

---

## 2.2 ADC Fundamentals (Days 3–5)

**Analog-to-Digital Conversion — understand every parameter:**
- Resolution: number of bits (12-bit = 4096 levels, 16-bit = 65536 levels)
- LSB (Least Significant Bit) size: `LSB = V_ref / 2^N` — calculate this for your Teensy (3.3V / 4096 = 0.806 mV for 12-bit)
- Quantization noise: the error introduced by rounding analog to digital
- Quantization noise power: `σ²_q = LSB² / 12` — this is the theoretical noise floor
- SNR of an ideal ADC: `SNR = 6.02 × N + 1.76 dB` (N = bits) — for 16-bit: ~98 dB
- ENOB (Effective Number of Bits): real ADCs are worse than ideal. ENOB = (SINAD - 1.76) / 6.02

**Sampling — this directly affects your bispectrum:**
- Sampling rate (fs): how many samples per second
- Nyquist frequency: `f_Nyquist = fs / 2` — the maximum frequency you can represent
- Nyquist theorem: you must sample at ≥ 2× the highest frequency in your signal
- Aliasing: if signal has energy above f_Nyquist, it folds back into your spectrum — CORRUPTS your bispectrum
- Anti-aliasing filter: analog low-pass filter BEFORE the ADC, cuts everything above f_Nyquist
- Your PRD doesn't mention anti-aliasing — you MUST add one (even a simple RC low-pass)

**Anti-aliasing filter design for SPECTER:**
- RC low-pass: `f_cutoff = 1 / (2π × R × C)`
- For 10 kHz sampling → Nyquist = 5 kHz → set cutoff at ~4 kHz (some margin)
- Example: R = 3.9kΩ, C = 10nF → f_c ≈ 4.08 kHz
- Place this between your sensor output and Teensy ADC input
- Higher-order filters (2nd order Sallen-Key) give sharper rolloff if needed

**ADC architectures (know conceptually):**
- SAR (Successive Approximation Register) — what Teensy uses, good speed/resolution tradeoff
- Delta-Sigma — very high resolution but slower, used in precision instruments
- Flash — fastest, lowest resolution, used in oscilloscopes

**ADC non-idealities — these are what SPECTER corrects:**
- Offset error: output is shifted by a constant
- Gain error: output scale factor is wrong
- DNL (Differential Nonlinearity): not all steps are equal size
- INL (Integral Nonlinearity): cumulative deviation from ideal straight line
- Missing codes: some digital values never appear
- These non-idealities are EXACTLY the nonlinear distortion your Volterra model captures

---

## 2.3 Sensor Interfaces (Days 5–8)

**SPI (Serial Peripheral Interface) — your Teensy-to-voltage-sensor bus:**
- 4-wire: SCLK (clock), MOSI (master out), MISO (master in), CS (chip select)
- Master generates clock — Teensy is master, sensor is slave
- Full duplex: data flows both directions simultaneously
- Clock polarity (CPOL) and clock phase (CPHA) — 4 modes (0,0), (0,1), (1,0), (1,1)
- You MUST match the sensor's expected SPI mode or you get garbage data
- Typical speeds: 1–50 MHz. Your PRD says 10 MHz — verify your sensor supports this
- Multiple slaves: each gets its own CS line, shared SCLK/MOSI/MISO
- DMA with SPI: configure DMA channel to automatically transfer SPI data to memory — no CPU intervention per byte

**I2C (Inter-Integrated Circuit) — your INA219 current sensor interface:**
- 2-wire: SDA (data), SCL (clock) — both open-drain with pull-up resistors
- Addressing: 7-bit address space (some devices support 10-bit)
- INA219 default address: 0x40 (configurable via A0, A1 pins)
- Speed modes: Standard (100 kHz), Fast (400 kHz), Fast-Plus (1 MHz)
- INA219 max effective sample rate: ~3.2 kHz at fastest conversion config — THIS IS SLOWER THAN YOUR 10 kHz SPI VOLTAGE SENSOR
- Pull-up resistor values: typically 2.2kΩ–4.7kΩ to VCC
- I2C debugging: use `i2cdetect -y 1` on Pi or scan loop on Teensy to find devices
- Common failures: wrong pull-up value, address conflict, bus stuck low (SDA held down)

**Sample rate mismatch problem (PRD gap you must solve):**
- Voltage sensor over SPI: up to 10+ kHz easily
- INA219 over I2C: max ~3.2 kHz
- Your cross-modal engine needs both at the same rate
- Solutions: (a) downsample voltage to match current rate, (b) upsample/interpolate current to match voltage rate, (c) run both at 3 kHz
- Recommended: run both at 3.2 kHz for simplicity during development, optimize later

**UART (Serial) — your Teensy-to-Pi data link:**
- Asynchronous: no shared clock, both sides must agree on baud rate
- Common baud rates: 9600, 115200, 921600, 1000000, 3000000
- USB Serial: Teensy emulates a USB CDC serial device — appears as `/dev/ttyACMx` on Pi
- USB serial speed: up to 12 Mbps (USB Full Speed on Teensy 4.1)
- Frame format: start bit + 8 data bits + optional parity + stop bit
- Your canonical protocol: `[sync:1B][timestamp:4B][voltage:2B][current:2B][checksum:1B]` = 10 bytes per sample

---

## 2.4 Grounding, Noise, and Power Integrity (Days 8–10)

**Why this matters: bad power/grounding = corrupted sensor data = broken calibration.**

**Ground loops:**
- What: when two ground connections create a loop that acts as an antenna, picking up interference
- Effect: 50/60 Hz hum injected into your sensor measurements
- Fix: single-point grounding (star topology), ground plane on PCB

**Decoupling capacitors:**
- What: small capacitors (100nF ceramic) placed close to each IC's power pins
- Why: absorb high-frequency noise spikes from digital switching
- Rule: 100nF on every single power pin, as close to the pin as physically possible
- Additional: 10μF electrolytic on power rail entry points

**Sensor noise sources you must understand:**
- Thermal noise (Johnson noise): `V_noise = √(4kTRΔf)` — exists in all resistors
- Shot noise: from discrete charge carriers — relevant in semiconductor sensors
- 1/f (flicker) noise: dominates at low frequencies — affects your DC and low-freq measurements
- Quantization noise: from ADC digitization (covered above)
- EMI (Electromagnetic Interference): from nearby motors, displays, switching regulators

**Practical shielding:**
- Use shielded cables for sensor connections if wires are > 10 cm
- Keep sensor analog signals away from digital switching lines (SPI clock, USB)
- Twisted pair for differential signals
- Ground guard rings on PCB around sensitive analog traces

---

## 2.5 Test Equipment Basics (Days 10–12)

**Multimeter — your first validation tool:**
- DC voltage measurement: verify voltage divider output matches calculation
- Resistance measurement: verify R_load value precisely (you need this for cross-modal)
- Continuity test: verify wiring connections
- Current measurement: ammeter in series (NEVER in parallel)

**Oscilloscope — your second validation tool:**
- Triggering: set trigger level and slope to capture stable waveform
- Timebase and voltage scale: adjust to see your signal clearly
- Measuring: frequency, period, amplitude, rise time
- AC coupling vs DC coupling — AC removes DC offset to see small variations
- You'll use scope to: verify SPI timing, measure jitter on Teensy acquisition loop, visualize sensor output noise

---

## MINI-PROJECTS — Phase 2

**Project 1: Voltage divider measurement rig**
- Build a resistive divider on breadboard (10kΩ + 10kΩ for 2:1 ratio)
- Apply known DC voltage (e.g., from bench supply or battery)
- Read with ADC (Arduino or Teensy)
- Calculate theoretical V_out, measure actual V_out
- Compute error in mV and percentage
- Vary temperature (hair dryer) and record how readings drift — THIS IS THE EXACT PROBLEM SPECTER SOLVES

**Project 2: INA219 read loop**
- Wire INA219 to known resistive load with known current
- Configure INA219 registers for fast conversion
- Poll at fixed rate (100 Hz), log timestamped values to serial
- Verify against multimeter reading
- Compute the offset error and gain error from your log data

**Project 3: SPI loopback test**
- Connect Teensy MOSI to MISO (loopback)
- Send known byte patterns at 10 MHz SPI clock
- Verify every received byte matches sent byte
- Measure timing with GPIO toggle + oscilloscope
- Count bit errors over 1 million bytes — should be zero

---

## EXIT CRITERIA — Phase 2

- [ ] Calculate ADC LSB size for any given reference voltage and bit count
- [ ] Explain why bad grounding corrupts calibration (in your own words)
- [ ] Debug a dead I2C bus: check pull-ups, scan addresses, verify wiring
- [ ] Design an RC anti-aliasing filter for a given sampling rate
- [ ] Explain the INA219 sample rate limitation and how it affects SPECTER
- [ ] Demonstrate voltage divider drift under temperature change with logged data

---

# PHASE 3 — Signals, DSP, and Numerical Foundations (Week 7–11)

This phase is the spine of bispectrum, Volterra, Wiener, and EKF. Every algorithm in SPECTER rests on these fundamentals.

---

## 3.1 Signals and Systems Fundamentals (Days 1–5)

**Continuous vs discrete signals:**
- Continuous-time: x(t), exists at every instant
- Discrete-time: x[n], exists only at integer indices n
- Sampling converts continuous to discrete: x[n] = x(nT), where T = 1/fs

**Signal properties you must identify on sight:**
- Periodic vs aperiodic
- Deterministic vs random (stochastic)
- Stationary vs non-stationary — a stationary signal has statistics that don't change over time. CRITICAL: the bispectrum assumes (at least weak) stationarity during each frame
- Ergodic vs non-ergodic — ergodic means time averages equal ensemble averages. Your bootstrap ASSUMES ergodicity during the 5-second collection window.

**Energy and power signals:**
- Energy signal: finite total energy (e.g., a pulse)
- Power signal: finite average power (e.g., a sine wave, a noise process)
- Sensor output is a power signal

**Linear Time-Invariant (LTI) systems:**
- Linear: `T[ax₁ + bx₂] = aT[x₁] + bT[x₂]` (superposition)
- Time-invariant: delaying the input delays the output by the same amount
- LTI systems are completely characterized by their impulse response h[n]
- Output = input convolved with impulse response: `y[n] = x[n] * h[n] = Σ h[k]·x[n-k]`
- Frequency domain equivalent: `Y(ω) = H(ω) · X(ω)` — multiplication, not convolution
- H(ω) is the transfer function — THE THING YOUR BISPECTRUM IS TRYING TO ESTIMATE BLINDLY
- SPECTER's sensor is modeled as a Wiener-Hammerstein system: LTI → nonlinearity → LTI. This is NOT fully LTI, which is why you need higher-order statistics.

**Causality and stability:**
- Causal: output depends only on present and past inputs (all physical systems are causal)
- BIBO stable: bounded input → bounded output. Requires Σ|h[n]| < ∞
- Your Volterra inverse filter MUST be stable or the corrected output explodes

---

## 3.2 Sampling Theory (Days 5–7)

**Nyquist-Shannon sampling theorem — know it cold:**
- Statement: a bandlimited signal with maximum frequency f_max can be perfectly reconstructed from samples taken at rate fs ≥ 2·f_max
- f_Nyquist = fs/2 — frequencies above this cannot be represented
- This is NOT just theory — violating it corrupts your bispectrum

**Aliasing — the enemy of blind identification:**
- What happens: frequency content above f_Nyquist "folds" back into the spectrum
- An alias at frequency f_alias = fs - f_signal is indistinguishable from a real signal at f_alias
- Your bispectrum will see aliased peaks as if they were real nonlinear products — WRONG identification
- Prevention: anti-aliasing filter (Phase 2.2) PLUS choosing fs high enough

**Reconstruction:**
- Ideal: sinc interpolation (impractical)
- Practical: zero-order hold (DAC), or software interpolation (linear, cubic)

---

## 3.3 Fourier Transform and FFT (Days 7–12)

**The Fourier Transform — conceptual understanding:**
- Any signal can be decomposed into a sum of sinusoids at different frequencies
- Continuous: `X(f) = ∫ x(t) · e^(-j2πft) dt`
- Discrete-Time Fourier Transform (DTFT): `X(ω) = Σ x[n] · e^(-jωn)` — continuous frequency, discrete time
- DFT (Discrete Fourier Transform): `X[k] = Σ x[n] · e^(-j2πkn/N)` for k = 0, 1, ..., N−1 — both discrete
- FFT is just a fast algorithm to compute the DFT in O(N log N) instead of O(N²)

**Complex numbers in DSP — you cannot avoid this:**
- `z = a + jb` where j = √(-1)
- Magnitude: `|z| = √(a² + b²)` — represents amplitude
- Phase: `∠z = atan2(b, a)` — represents time delay
- Conjugate: `z* = a - jb` — used everywhere (bispectrum formula, Wiener filter)
- Euler's formula: `e^(jθ) = cos(θ) + j·sin(θ)` — this IS the Fourier basis function
- `np.abs()` for magnitude, `np.angle()` for phase, `np.conj()` for conjugate

**FFT practical details:**
- Input: N samples → Output: N complex frequency bins
- Bin spacing: `Δf = fs / N` — frequency resolution
- For N=512, fs=10kHz → Δf = 19.5 Hz per bin
- Bins 0 to N/2: positive frequencies (0 to fs/2)
- Bins N/2+1 to N-1: negative frequencies (mirror for real signals)
- For real input: spectrum is symmetric — only first N/2+1 bins are unique
- `np.fft.fft()` returns complex array, `np.fft.fftfreq(N, 1/fs)` gives frequency axis
- `np.fft.rfft()` returns only positive frequencies for real input — use this for efficiency

**Power Spectral Density (PSD):**
- `PSD[k] = |X[k]|² / N` — power at each frequency
- Welch's method: average PSD across overlapping windowed segments — reduces variance
- `scipy.signal.welch()` — use this, don't implement from scratch
- PSD tells you WHERE the signal energy is. Bispectrum tells you HOW frequencies INTERACT.

**Windowing — why it exists and which to use:**
- Problem: DFT assumes the signal is periodic over N samples. If it's not, the discontinuity at the edges creates spectral leakage — fake energy smeared across all bins
- Window functions multiply the signal to taper edges to zero
- Hanning (Hann) window: `w[n] = 0.5 × (1 − cos(2πn/N))` — THIS IS WHAT SPECTER USES
- Why Hanning: good trade-off between main lobe width and side lobe suppression
- Other windows: Hamming (slightly less side-lobe), Blackman (more suppression, wider main lobe), Rectangular (no windowing = maximum leakage)
- Always window before FFT in your pipeline
- Amplitude correction: windowing reduces signal amplitude. Apply scaling factor: `2.0 / sum(window)` for amplitude-accurate spectrum

**Spectral leakage — see it with your own eyes:**
- Generate a sine at frequency that is NOT an exact multiple of Δf
- FFT without windowing → energy spreads across many bins (leakage)
- FFT with Hanning window → energy concentrated near the true bin
- This is your first mini-project validation

---

## 3.4 FIR and IIR Filters (Days 12–15)

**FIR (Finite Impulse Response) — SPECTER's Wiener equalizer is a 64-tap FIR:**
- Output: `y[n] = Σ h[k] · x[n-k]` for k = 0 to M-1 (M = number of taps)
- "Finite" because the impulse response has finite length M
- Always stable (no feedback, no poles)
- Can have exactly linear phase (symmetric coefficients) — preserves waveform shape
- Higher M = narrower transition band = more computation
- Implementation: `np.convolve(x, h)` in Python, manual MAC loop in C

**FIR filter design methods:**
- Window method: start with ideal frequency response, take IFFT, multiply by window
- `scipy.signal.firwin(numtaps, cutoff, fs=fs)` — designs low-pass FIR
- `scipy.signal.firwin(numtaps, [f_low, f_high], pass_zero=False, fs=fs)` — band-pass
- Remez exchange algorithm: `scipy.signal.remez()` — optimal equiripple design
- Know that your 64-tap Wiener equalizer coefficients come from IFFT of W(ω), not from firwin

**IIR (Infinite Impulse Response) — know the differences:**
- Has feedback: `y[n] = Σ b[k]·x[n-k] - Σ a[k]·y[n-k]`
- "Infinite" because impulse response never truly reaches zero
- Can be unstable (poles outside unit circle)
- More efficient (fewer coefficients for same sharpness) but nonlinear phase
- Common types: Butterworth (maximally flat), Chebyshev (sharper but ripple), Elliptic (sharpest but ripple in both bands)
- SPECTER uses FIR for the equalizer, not IIR — because FIR guarantees stability and the coefficients are directly from the IFFT

**Convolution — the fundamental operation:**
- `y[n] = (x * h)[n] = Σ h[k]·x[n-k]`
- Convolution in time domain = multiplication in frequency domain (and vice versa)
- This duality is WHY the Wiener equalizer works: multiply in frequency domain, IFFT to get FIR taps
- Linear convolution vs circular convolution — FFT computes circular. For linear, zero-pad to length ≥ len(x) + len(h) - 1
- Overlap-save / overlap-add methods: efficient block-based convolution using FFT — relevant if you move filtering to frequency domain

---

## 3.5 Linear Algebra for DSP (Days 15–20)

**Matrix operations — these ARE your algorithm implementations:**
- Matrix multiply: `C = A @ B` in NumPy. Know the dimension rules: (m×n) @ (n×p) = (m×p)
- Transpose: `A.T` — rows become columns
- Conjugate transpose (Hermitian): `A.conj().T` — for complex matrices. Critical for `H*(ω)` in Wiener filter.
- Identity matrix: `np.eye(n)` — multiplying by identity does nothing
- Inverse: `np.linalg.inv(A)` — A⁻¹ such that A @ A⁻¹ = I. AVOID explicit inverse when possible.
- Determinant: `np.linalg.det(A)` — if near zero, matrix is near-singular

**Solving linear systems — you'll do this for Fredholm, EKF, and Volterra:**
- Problem: find x such that `A @ x = b`
- `np.linalg.solve(A, b)` — uses LU decomposition, O(n³) — preferred over computing A⁻¹
- `scipy.linalg.solve(A, b)` — LAPACK-backed, slightly faster, more options
- Overdetermined systems (more equations than unknowns): `np.linalg.lstsq(A, b)` — least squares

**Condition number — critical for numerical stability:**
- `cond(A) = σ_max / σ_min` (ratio of largest to smallest singular value)
- Low condition number (~1): well-conditioned, solution is reliable
- High condition number (>10⁶): ill-conditioned, small input changes → huge output changes
- `np.linalg.cond(A)` — check this for your Fredholm system matrix
- Rule of thumb: you lose log₁₀(cond(A)) digits of accuracy in the solution
- If cond = 10⁸ and you're using float64 (~15 digits), you have ~7 digits of accuracy left

**Eigenvalues and eigenvectors:**
- `A @ v = λ · v` — v is eigenvector, λ is eigenvalue
- `np.linalg.eig(A)` — returns eigenvalues and eigenvectors
- Eigenvalues tell you about system modes, stability, and conditioning
- For symmetric/Hermitian matrices: `np.linalg.eigh()` — faster, guaranteed real eigenvalues

**SVD (Singular Value Decomposition):**
- `A = U @ Σ @ V^H` — U and V are orthogonal, Σ is diagonal with singular values
- `np.linalg.svd(A)`
- Singular values: like eigenvalues but for non-square matrices
- SVD is the most robust way to compute condition number, rank, and pseudo-inverse
- Truncated SVD: keep only the top k singular values — used for regularized solutions

**Tikhonov regularization — you MUST understand this for SPECTER:**
- Problem: `A @ x = b` but A is ill-conditioned → solution is garbage
- Regularized problem: `minimize ||A @ x - b||² + λ² · ||x||²`
- Solution: `x = (A^T @ A + λ² · I)⁻¹ @ A^T @ b`
- Or equivalently: `np.linalg.lstsq(np.vstack([A, lambda_reg * np.eye(n)]), np.concatenate([b, np.zeros(n)]))`
- λ (regularization parameter): too small = no effect, too large = over-smoothed solution
- How to choose λ: L-curve method (plot ||x|| vs ||Ax-b|| for different λ, pick the "knee")
- THIS is what saves your Fredholm equation solve from exploding

---

## 3.6 Probability and Statistics for Signals (Days 20–25)

**Random variables and distributions:**
- PDF (probability density function): describes likelihood of each value
- Gaussian (Normal) distribution: `N(μ, σ²)` — defined by mean μ and variance σ²
- Central Limit Theorem: sum of many independent random variables → Gaussian. THIS is why sensor noise is often approximately Gaussian.
- Uniform distribution: all values equally likely — used for quantization noise model

**Moments and cumulants — the math behind the bispectrum:**
- Mean (1st moment): `μ = E[x]`
- Variance (2nd central moment): `σ² = E[(x-μ)²]`
- Skewness (3rd standardized moment): `γ = E[(x-μ)³] / σ³` — measures asymmetry
- Kurtosis (4th standardized moment): `κ = E[(x-μ)⁴] / σ⁴ - 3` — measures tail heaviness (excess kurtosis; Gaussian = 0)

**Cumulants vs moments:**
- Moments: E[x], E[x²], E[x³], E[x⁴]
- Cumulants: related to moments but have a CRITICAL property: **all cumulants of order ≥ 3 are ZERO for a Gaussian process**
- This is WHY the bispectrum works: the bispectrum is the Fourier transform of the 3rd-order cumulant. If the input is Gaussian and the sensor is linear, bispectrum = 0. Any non-zero bispectrum = evidence of nonlinearity. That's the entire theoretical lever.

**Autocorrelation and power spectrum:**
- Autocorrelation: `R_xx[k] = E[x[n] · x[n+k]]` — how similar a signal is to a delayed version of itself
- Wiener-Khinchin theorem: PSD = Fourier transform of autocorrelation
- This is 2nd-order statistics. Your standard power spectrum lives here. It CANNOT detect nonlinearity.

**Cross-correlation:**
- `R_xy[k] = E[x[n] · y[n+k]]`
- Used to measure similarity between two signals at different lags
- Relevant for verifying cross-modal consistency

**Estimation vs true values:**
- Sample mean vs true mean — sample mean has variance that decreases with N
- Bias and consistency of estimators
- Your bispectrum is an ESTIMATE from finite data — it has variance. More frames M = lower variance = better estimate. THIS is why M=100 might not be enough.

---

## MINI-PROJECTS — Phase 3

**Project 1: FFT lab**
- Generate a sine at 440 Hz + harmonic at 880 Hz + noise. Sample at 8 kHz.
- FFT with rectangular window: observe leakage
- FFT with Hanning window: observe clean peaks
- Plot both spectra (magnitude in dB vs frequency in Hz)
- Identify the exact bin indices and amplitudes

**Project 2: FIR equalization sandbox**
- Create a "channel" distortion: a 32-tap FIR that rolls off above 500 Hz
- Pass a broadband signal through it
- Design an inverse FIR using frequency inversion: `W(ω) = 1/H(ω)` → IFFT → truncate
- Apply the inverse filter and show the signal is recovered
- Then add noise and show that without regularization (λ=0), the inverse blows up the noise
- Add Wiener-style regularization (`W(ω) = H*(ω)/(|H(ω)|² + λ)`) and show stable recovery

**Project 3: Ill-conditioning demo**
- Create a near-singular 10×10 matrix (Hilbert matrix: `scipy.linalg.hilbert(10)`)
- Compute its condition number — should be ~10¹³
- Solve `A @ x = b` for known x — show the solved x has huge errors
- Apply Tikhonov regularization — show the error drops dramatically
- Plot: λ vs solution error, λ vs residual — find the optimal λ (L-curve)

---

## EXIT CRITERIA — Phase 3

- [ ] Explain why the bispectrum can detect nonlinearity but the power spectrum cannot (in your own words)
- [ ] Derive and code a 64-tap FIR from its frequency response (IFFT method)
- [ ] Detect aliasing in a plot without guessing — point to the aliased frequency
- [ ] Explain what condition number means and why it matters for Fredholm solve
- [ ] Stabilize an ill-conditioned solve with Tikhonov regularization
- [ ] Compute a PSD using Welch's method and explain every parameter
- [ ] Explain stationarity and ergodicity and why SPECTER's bootstrap assumes both

---

# PHASE 4 — Embedded Systems Core (Week 12–15)

Teensy is your real-time anchor. If this is weak, the whole system is unstable.

---

## 4.1 Microcontroller Architecture (Days 1–4)

**ARM Cortex-M7 (Teensy 4.1) — know your hardware:**
- Harvard architecture: separate instruction and data buses — faster than Von Neumann
- 32-bit ARM, runs at 600 MHz, double-precision FPU
- 1 MB SRAM (tightly coupled: 512 KB DTCM + 512 KB ITCM) + 16 MB external PSRAM
- Flash: 8 MB (program storage)
- DTCM (Data Tightly Coupled Memory): zero wait-state access — put your buffers here
- ITCM (Instruction Tightly Coupled Memory): zero wait-state — put your hot-path code here
- Cache: 32 KB I-cache + 32 KB D-cache for external memory access

**Registers and peripherals:**
- GPIO registers: configure pins as input/output, set/clear/read pin values
- Each peripheral (SPI, I2C, UART, ADC, Timer) has a set of memory-mapped configuration registers
- Teensy/Arduino API abstracts these, but you must understand what happens underneath
- `digitalWriteFast()` vs `digitalWrite()` — direct register access vs abstracted function call

**Clock tree:**
- ARM_CLK: 600 MHz (CPU)
- IPG_CLK: 150 MHz (peripheral bus)
- SPI clock derives from peripheral clock — affects max SPI speed
- Timer clock derives from peripheral clock — affects sample timing resolution
- PLL (Phase-Locked Loop): multiplies crystal frequency to get 600 MHz

**Memory map:**
- Flash starts at a specific address, SRAM at another
- Stack grows downward from top of SRAM
- Heap grows upward from end of static data
- Your ring buffers compete with stack for SRAM — size carefully

---

## 4.2 Interrupts (Days 4–7)

**Interrupt model — the core of real-time:**
- Interrupt: hardware signal that stops current code and runs an ISR (Interrupt Service Routine)
- ISR: a function that runs in response to an interrupt — must be SHORT
- NVIC (Nested Vectored Interrupt Controller): ARM's interrupt management hardware
- Priority levels: lower number = higher priority. Teensy has configurable priorities.
- Nesting: a higher-priority interrupt can preempt a lower-priority ISR
- Interrupt latency: cycles between interrupt assertion and first ISR instruction — Cortex-M7: 12 cycles typical

**ISR safety rules — memorize these:**
- Do NOT allocate memory (no `malloc`, no `new`)
- Do NOT use `Serial.print()` or any I/O that blocks
- Do NOT call `delay()` or any blocking function
- Do NOT run complex math — compute offline, ISR just flags or copies data
- Do NOT hold locks for long — risk of priority inversion
- Keep ISR as short as possible: set a flag, copy data, exit. Process in `loop()`.

**Volatile keyword:**
- `volatile uint8_t flag = 0;` — tells compiler: don't optimize this variable, it can change outside normal program flow (i.e., in an ISR)
- Without `volatile`, compiler may cache the variable in a register and never re-read it from memory
- ALL variables shared between ISR and main code MUST be `volatile`

**Critical sections:**
- `noInterrupts();` / `interrupts();` — disable/enable all interrupts
- Use to protect shared data from corruption during multi-byte reads/writes
- Keep critical sections as short as possible — interrupts disabled = missed events

---

## 4.3 Timer Hardware and Deterministic Sampling (Days 7–10)

**Hardware timers — how you achieve <100 ns jitter:**
- Teensy 4.1 has multiple hardware timer channels (PIT, GPT, QTMR)
- PIT (Periodic Interrupt Timer): simplest, generates interrupt at fixed intervals
- Configure IntervalTimer/PIT to fire at your synchronized sample rate (312.5 μs for 3.2 kHz)
- ISR is triggered by timer, not by `delay()` — this is what makes timing deterministic

**Jitter measurement — prove your timing:**
- In timer ISR: toggle a GPIO pin
- Measure pin toggle period with oscilloscope
- Jitter = variation in toggle period across 1000+ measurements
- Target: < 100 ns jitter (PRD requirement)
- If jitter > 100 ns: check for interrupt priority conflicts, disable other interrupts during timing-critical ISR

**IntervalTimer library (Teensy-specific):**
- `IntervalTimer timer; timer.begin(sampleISR, period_microseconds);`
- Higher priority than most other interrupts
- Use this for your acquisition clock

**Why `delay()` is forbidden:**
- `delay(1000)` blocks the CPU for 1000 ms — nothing else runs
- Cannot achieve deterministic timing with blocking delays
- Use timer interrupts for periodic tasks, `millis()` for non-critical timing in `loop()`

---

## 4.4 DMA (Direct Memory Access) (Days 10–13)

**What DMA is:**
- A hardware engine that copies data between peripherals and memory WITHOUT CPU involvement
- CPU sets up the transfer (source, destination, count), starts it, then does other work
- When transfer completes, DMA generates an interrupt
- Critical for high-throughput: CPU is free to compute while DMA handles data movement

**SPI + DMA on Teensy:**
- Configure SPI peripheral to trigger DMA on each received byte
- DMA automatically writes received SPI bytes to a memory buffer
- No CPU intervention per byte — zero per-sample overhead
- Teensy DMA library: use `DMAChannel` class to configure

**Ping-pong double buffering — WHY you need it:**
- Problem: if you read ADC samples into a buffer AND process that buffer simultaneously, you get race conditions (partially overwritten data)
- Solution: two buffers (A and B)
- While DMA fills buffer A → CPU processes buffer B
- When A is full: swap roles — DMA fills B, CPU processes A
- Swap is triggered by DMA completion interrupt
- Implementation: `volatile uint8_t active_buffer = 0;` — ISR toggles, main loop reads opposite

**Circular/ring buffers — for streaming data:**
- Fixed-size array with read and write pointers that wrap around
- `write_idx = (write_idx + 1) % BUFFER_SIZE`
- `read_idx = (read_idx + 1) % BUFFER_SIZE`
- Empty: `read_idx == write_idx`
- Full: `(write_idx + 1) % BUFFER_SIZE == read_idx`
- Use for: USB serial transmit buffer, sample history for filtering
- Power-of-2 buffer sizes: use `& (SIZE-1)` instead of `% SIZE` — faster

---

## 4.5 USB Serial Communication (Days 13–16)

**USB CDC (Communications Device Class):**
- Teensy 4.1 appears as a virtual serial port over USB
- On Pi: shows up as `/dev/ttyACMx`
- Speed: up to 12 Mbps (USB Full Speed) — faster than UART
- No baud rate configuration needed (USB handles framing)

**Binary protocol design — your Teensy-to-Pi data link:**
- Canonical packet format: `[sync:1B][timestamp_us:4B][voltage_raw:2B][current_raw:2B][checksum:1B]` = 10 bytes
- Timestamp: 32-bit microsecond counter from `micros()` — wraps every ~71 minutes
- Voltage: 16-bit signed int from ADC
- Current: 16-bit signed int from INA219
- Checksum: XOR of all preceding bytes, or CRC-8 for stronger error detection

**Framing — how the Pi finds packet boundaries in a byte stream:**
- Option 1: fixed-size packets (10 bytes) with sync byte as byte 0 (`0xAA`) in every packet
- Option 2: COBS (Consistent Overhead Byte Stuffing) — encodes packets so 0x00 never appears in data, use 0x00 as delimiter
- Option 3: SLIP (Serial Line Internet Protocol) — similar to COBS, uses escape sequences
- For SPECTER: fixed-size packets with sync byte + checksum is simplest and sufficient

**Handling data integrity:**
- Teensy: compute checksum before sending
- Pi: validate checksum on receive — drop corrupt packets silently
- Count dropped packets — if > 1% you have a wiring or timing problem
- Frame synchronization: if stream gets misaligned, scan for next sync byte

**Endianness:**
- Teensy (ARM): little-endian — least significant byte first
- Pi (ARM): little-endian — same. No byte swap needed.
- If you ever port to a big-endian system: use `htons()`/`ntohs()` or manually swap

---

## 4.6 Teensy Toolchain and Development (Days 16–18)

**PlatformIO (recommended) or Arduino IDE:**
- PlatformIO: command-line build system, integrates with VS Code
- `platformio.ini`: board = teensy41, framework = arduino, upload_port = /dev/ttyACMx
- Build: `pio run`, Upload: `pio run -t upload`, Monitor: `pio device monitor`
- Arduino IDE: simpler but less flexible — fine for early prototyping

**Profiling and timing on Teensy:**
- `ARM_DWT_CYCCNT` — cycle counter register, incremented every CPU clock (600 MHz = 1.67 ns resolution)
- Enable: `ARM_DEMCR |= ARM_DEMCR_TRCENA; ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;`
- Use to measure exact cycle count of any function:
  ```cpp
  uint32_t start = ARM_DWT_CYCCNT;
  doWork();
  uint32_t cycles = ARM_DWT_CYCCNT - start;
  float us = cycles / 600.0f;  // Convert to microseconds
  ```
- This is how you prove your hot-path meets timing budget

---

## MINI-PROJECTS — Phase 4

**Project 1: Timer-driven acquisition loop**
- Configure IntervalTimer to fire at 3.2 kHz (312.5 μs period)
- In ISR: read ADC, toggle GPIO pin, write sample to ring buffer
- In `loop()`: if buffer has data, print to serial
- Measure GPIO toggle period with oscilloscope — verify < 100 ns jitter
- Profile ISR execution time with `ARM_DWT_CYCCNT`

**Project 2: DMA ping-pong buffer**
- Configure SPI + DMA to fill a 512-sample buffer
- Implement double-buffering: DMA fills A while CPU processes B
- Processing = compute mean and max of buffer (simple, just to verify)
- On DMA complete interrupt: swap buffers, set processing flag
- Verify: no samples dropped over 60 seconds of continuous operation

**Project 3: Binary serial protocol**
- Teensy sends packets: `[0xAA][timestamp:4B][voltage:2B][current:2B][XOR_checksum:1B]`
- Pi Python receiver: reads serial, finds sync byte, extracts fields, validates checksum
- Count: total packets, valid packets, corrupted packets, sync losses
- Run for 60 seconds at 3.2 kHz — target: 0 corrupted packets, 0 sync losses

---

## EXIT CRITERIA — Phase 4

- [ ] Maintain stable 3.2 kHz sample timing with < 100 ns jitter (oscilloscope proof)
- [ ] Explain ISR safety rules from memory — what you can and cannot do in an ISR
- [ ] Implement DMA ping-pong buffer with zero sample loss over 60 seconds
- [ ] Implement checksum validation end-to-end (Teensy → Pi)
- [ ] Measure ISR execution time in CPU cycles using `ARM_DWT_CYCCNT`
- [ ] Explain volatile keyword and why it's needed for ISR-shared variables

---

# PHASE 5 — Python System Orchestration (Week 16–18)

Your Pi side must remain stable while running heavy DSP and streaming telemetry.

---

## 5.1 Multiprocessing Architecture (Days 1–4)

**Why multiprocessing, not asyncio (for compute-heavy tasks):**
- Python GIL (Global Interpreter Lock): only one thread can execute Python bytecode at a time
- `threading` module: threads share GIL — no true parallelism for CPU-bound work
- `asyncio`: cooperative multitasking in ONE thread — great for I/O wait, terrible for CPU math
- `multiprocessing`: spawns separate Python processes, each with its own GIL — TRUE parallelism
- Pi 5 has 4 cores — you can run 4 processes simultaneously

**Process design for SPECTER:**
- Process 1 (Acquisition): reads USB serial, writes to shared ring buffer
- Process 2 (Compensation): applies Volterra + Wiener filter continuously
- Process 3 (EKF/CUSUM): runs state estimation at 100 Hz
- Process 4 (Telemetry): packages data, sends to ESP32 over UART

**`multiprocessing` module essentials:**
- `Process(target=func, args=(...))` — spawn a process
- `.start()`, `.join()`, `.is_alive()`, `.terminate()`
- `Queue` — FIFO inter-process communication (thread-safe, process-safe)
- `Queue.put(item)`, `Queue.get(timeout=1.0)` — with timeout to prevent blocking forever
- `Queue` has serialization overhead (pickle) — avoid for high-rate data
- `Event` — simple signaling between processes (set/wait/clear)
- `Lock` — mutual exclusion for shared resources

**`multiprocessing.shared_memory` — for coefficient handoff:**
- Creates a shared memory block accessible by multiple processes — no serialization
- `shm = SharedMemory(name='coefficients', create=True, size=num_bytes)`
- Wrap with NumPy: `arr = np.ndarray(shape, dtype=np.float64, buffer=shm.buf)`
- Process 3 (EKF) writes updated coefficients → Process 2 (Compensation) reads them
- WARNING: no built-in synchronization — use a `Lock` or atomic flag to prevent reading half-written data

---

## 5.2 Serial Communication on Pi (Days 4–6)

**pyserial — your Teensy data receiver:**
- `import serial; ser = serial.Serial('/dev/ttyACM0', timeout=0.1)`
- For USB CDC: baud rate parameter is ignored (USB handles speed)
- `ser.read(10)` — read exactly 10 bytes (one packet)
- `ser.in_waiting` — number of bytes available in input buffer
- `ser.reset_input_buffer()` — flush stale data on startup

**Efficient reading loop:**
```python
while True:
    if ser.in_waiting >= 10:
        raw = ser.read(10)
        if raw[0] != 0xAA:  # sync byte check
            # resync: scan for next 0xAA
            continue
        if validate_checksum(raw):
            sync, timestamp, voltage, current, chk = struct.unpack('<BIhhB', raw)
            # unpacked: little-endian sync + uint32 + int16 + int16 + checksum
```
- `struct.unpack()` — converts raw bytes to Python integers using format string
- Format codes: `<` (little-endian), `B` (uint8), `H` (uint16), `h` (int16), `I` (uint32), `f` (float32), `d` (float64)

---

## 5.3 NumPy Performance Patterns (Days 6–8)

**Vectorization — the #1 rule:**
- BAD: `for i in range(len(x)): y[i] = x[i] * h[i]` — Python loop, ~100× slower
- GOOD: `y = x * h` — NumPy vectorized, executes in C underneath
- CRITICAL: all SPECTER hot-path operations must be vectorized

**Memory layout and copies:**
- `np.array(x, copy=False)` — avoid unnecessary copies
- `x.flags` — check if array is C-contiguous (row-major) or Fortran-contiguous (column-major)
- `np.ascontiguousarray(x)` — force C-contiguous for optimal cache performance
- Slicing creates views (no copy): `y = x[10:20]` — modifying y modifies x
- `.copy()` — force a copy when you need independence

**Pre-allocating output arrays:**
- BAD: appending to list, then converting to array
- GOOD: `output = np.empty(N, dtype=np.float64); output[i] = ...` — pre-allocate
- `np.empty()` — allocation without initialization (fastest)
- `np.zeros()` — allocation with zero-fill

**Profiling:**
- `time.perf_counter_ns()` — nanosecond resolution timer
- `%timeit` in IPython — automatic benchmarking with multiple runs
- `cProfile` — function-level profiling: `python -m cProfile -s cumtime your_script.py`
- `line_profiler` — line-by-line timing: `@profile` decorator + `kernprof`

---

## 5.4 Structured Logging and Telemetry (Days 8–10)

**Logging (not print statements):**
- `import logging; logging.basicConfig(level=logging.INFO)`
- Levels: `DEBUG`, `INFO`, `WARNING`, `ERROR`, `CRITICAL`
- Format: `%(asctime)s [%(levelname)s] %(name)s: %(message)s`
- Log to file: `logging.FileHandler('specter.log')`
- Rotating logs: `RotatingFileHandler(maxBytes=10*1024*1024, backupCount=5)`

**JSON telemetry schema:**
```python
telemetry_frame = {
    "timestamp_ms": int,
    "voltage_raw": float,
    "voltage_corrected": float,
    "current_raw": float,
    "current_corrected": float,
    "crossmodal_error": float,
    "ekf_state_norm": float,
    "cusum_value": float,
    "confidence_score": int,
    "system_state": str,  # "BOOTSTRAP", "COMPENSATING", etc.
}
```
- Serialize: `json.dumps(frame).encode('utf-8')`
- Send to ESP32 over UART at 10 Hz

---

## 5.5 Process Lifecycle and Crash Recovery (Days 10–12)

**Supervisor pattern:**
- Main process spawns workers, monitors them in a loop
- If `process.is_alive()` returns False: log the crash, restart the process
- Preserve last-known-good state in shared memory (coefficients survive worker restart)

**Graceful shutdown:**
- Use `Event` objects: `shutdown_event = Event()`
- Workers check `while not shutdown_event.is_set():` in their main loop
- Main process: `shutdown_event.set()` on SIGINT (Ctrl-C)
- `signal.signal(signal.SIGINT, handler)` — register the signal handler

**State persistence:**
- On each EKF update: optionally write state vector to JSON file
- On restart: load last-known-good state from JSON
- This enables "resume after reboot" (mentioned in PRD for `kalman.py`)

---

## MINI-PROJECTS — Phase 5

**Project 1: 3-process pipeline**
- Process A: reads mock serial data (or real Teensy), pushes samples to Queue
- Process B: reads Queue, applies a simple FIR filter, writes results to shared memory
- Process C: reads shared memory at 10 Hz, prints telemetry JSON
- Verify all 3 processes actually run on different CPU cores (`os.getpid()`, `htop`)

**Project 2: Shared memory coefficient handoff**
- Process A: writes a 64-element float64 array to shared memory every 100 ms
- Process B: reads the array continuously, computes dot product with a signal
- Use a Lock to prevent partial reads
- Inject a deliberate race condition (remove lock) — observe corrupted reads. Then fix.

**Project 3: Crash recovery supervisor**
- Worker process that crashes randomly (raise exception after random delay)
- Supervisor detects crash, logs it, restarts worker within 1 second
- Verify: supervisor survives 10 worker crashes in a row without dying
- Last-known-good state preserved across crashes

---

## EXIT CRITERIA — Phase 5

- [ ] Prove processes run in parallel (show multi-core utilization in htop)
- [ ] Hot-swap coefficients in shared memory without pausing the compensation stream
- [ ] Recover from worker crash automatically within 1 second
- [ ] Serialize/deserialize a telemetry frame to JSON correctly
- [ ] Profile NumPy operations and identify which are the bottleneck
- [ ] Handle Ctrl-C gracefully — all processes shut down cleanly

---

# PHASE 6 — SPECTER Algorithm Track (Week 19–24)

This is where you implement the actual DSP stack. Each stage builds on the previous.

---

## 6.1 Stage A — Bispectrum Estimation (Week 19–20)

### Learn — every detail

**Third-order cumulants:**
- 2nd-order cumulant = covariance: `c_2(τ) = E[x(n)·x(n+τ)] - μ²`
- 3rd-order cumulant: `c_3(τ₁, τ₂) = E[x(n)·x(n+τ₁)·x(n+τ₂)]` (for zero-mean signals)
- For Gaussian signals: c_3 = 0 identically. This is the foundational property.
- Non-zero c_3 = evidence of non-Gaussianity = evidence of nonlinearity in the sensor

**Bispectrum definition:**
- `B(ω₁, ω₂) = ∫∫ c_3(τ₁, τ₂) · e^(-j(ω₁τ₁ + ω₂τ₂)) dτ₁ dτ₂`
- Discrete: `B̂(k₁, k₂) = X(k₁) · X(k₂) · X*(k₁ + k₂)`
- X(k) = DFT of windowed frame
- X*(k) = complex conjugate of X(k)
- Result is a 2D complex-valued array indexed by frequency pairs (k₁, k₂)

**Principal domain — reduce computation by 12×:**
- Due to symmetries of the bispectrum (Hermitian, triangular), you only need to compute the region: `0 ≤ k₂ ≤ k₁`, `k₁ + k₂ ≤ N/2`
- This triangular wedge contains all unique information
- Implement: nested loop over k₁ and k₂ with boundary checks, or precompute valid index pairs

**Frame averaging for convergence:**
- Single-frame bispectrum is extremely noisy
- Average over M frames: `B̄(k₁, k₂) = (1/M) Σ B̂_m(k₁, k₂)`
- Variance decreases as 1/M — need large M for stable estimate
- PRD says M=100. Test empirically: plot bispectrum at M=10, 50, 100, 200, 500 for YOUR sensor
- Define "convergence": relative change between M and M+50 < some threshold (e.g., 5%)

**The critical assumption — input must be approximately Gaussian:**
- Bispectrum of Gaussian input + linear system = 0
- Bispectrum of Gaussian input + nonlinear system ≠ 0 → reveals nonlinearity
- If input is NOT Gaussian (e.g., periodic square wave), the input itself produces bispectral peaks → you can't distinguish sensor nonlinearity from input non-Gaussianity
- For SPECTER: sensor excitation should be broadband, noisy, approximately Gaussian
- Thermal noise on a resistive divider: good (approximately Gaussian)
- Ultrasonic ranger output: bad (discrete, periodic — violates assumption)

### Build

- Generate synthetic test signal: `x = np.random.randn(N)` through a known nonlinear function `y = x + 0.3*x² + 0.1*x³ + noise`
- Window each 512-sample frame with Hanning
- Compute 512-point FFT of each frame
- Compute `B̂(k₁, k₂) = X(k₁) * X(k₂) * np.conj(X(k₁ + k₂))` in principal domain
- Average over M frames
- Plot as 2D heatmap: `plt.imshow(np.abs(B_avg), aspect='auto')` — should show diagonal ridges from the x² term
- Compare: repeat with linear system (no x², x³) — bispectrum should be flat/near-zero
- Convergence study: plot ||B̄ₘ - B̄ₘ₋₁|| vs M — find where it plateaus

### Exit

- [ ] Show bispectrum convergence plots for your synthetic signal
- [ ] Justify chosen M value with empirical evidence
- [ ] Demonstrate that a linear system has near-zero bispectrum
- [ ] Explain the Gaussianity assumption and what breaks it

---

## 6.2 Stage B — Volterra Kernel Extraction and Inverse (Week 20–21)

### Learn — every detail

**Volterra series — the mathematical form:**
- 1st-order (linear): `y₁[n] = Σ h₁[τ]·x[n-τ]` — this is just an FIR filter
- 2nd-order: `y₂[n] = ΣΣ h₂[τ₁, τ₂]·x[n-τ₁]·x[n-τ₂]`
- h₁[τ]: 1D kernel (impulse response) — captures linear dynamics
- h₂[τ₁, τ₂]: 2D kernel — captures quadratic nonlinear interactions with memory
- Truncation to 2nd order captures ~95% of nonlinear energy for most MEMS sensors
- Full output: `y[n] = h₀ + y₁[n] + y₂[n] + higher order (ignored)`

**Relationship between bispectrum and Volterra kernels:**
- Linear kernel in frequency domain: `H₁(ω)` estimated from power spectrum ratio
- Quadratic kernel: `H₂(ω₁, ω₂) = B_y(ω₁, ω₂) / [H₁(ω₁)·H₁(ω₂)·H₁*(ω₁+ω₂)·S_x(ω₁)·S_x(ω₂)]`
- S_x(ω) = input power spectrum — UNKNOWN. Assumed white: S_x(ω) = σ²_x (constant)
- This assumption introduces estimation bias — acknowledge it, document it, test sensitivity
- H₁(ω) estimation: from ratio of output power spectrum to assumed input spectrum

**Fredholm equation discretization:**
- The inverse kernel satisfies a 2D integral equation (Fredholm equation of the second kind)
- Discretize: convert integral to matrix equation `A @ x = b`
- A is typically dense and ill-conditioned — condition number may be 10⁶–10¹²
- Size: if using L memory taps, A is (L²×L²) — for L=16: A is 256×256

**Regularization of the solve:**
```python
lambda_reg = 1e-4  # tune this
A_reg = A.T @ A + lambda_reg * np.eye(A.shape[1])
b_reg = A.T @ b
h2_inv = np.linalg.solve(A_reg, b_reg)
```
- Start with λ=1e-4, test from 1e-6 to 1e-2 — use L-curve if needed
- Alternative: `np.linalg.lstsq(np.vstack([A, lambda_reg*np.eye(n)]), np.concatenate([b, np.zeros(n)]))`

**Inverse Volterra filter — applying the correction:**
- Once you have the inverse kernels h₁_inv and h₂_inv:
```python
y_corrected = np.convolve(y_raw, h1_inv, mode='same')
# Add 2nd-order correction:
for tau1 in range(L):
    for tau2 in range(L):
        y_corrected[n] += h2_inv[tau1, tau2] * y_raw[n-tau1] * y_raw[n-tau2]
```
- In production: vectorize the 2nd-order term using NumPy outer products
- L=16 dominant coefficients: 256 multiply-accumulate ops per sample — feasible on Teensy

### Build

- Create synthetic test: known linear filter H₁ + quadratic nonlinearity f(x)=x+0.3x²
- Pass white Gaussian noise through it to create synthetic sensor output
- Run your bispectrum estimation (Stage A) → extract H₁ and H₂
- Solve for inverse kernels with Tikhonov regularization
- Apply inverse filter to synthetic distorted signal
- Measure: MSE before and after correction — should drop by 10×+
- Test stability: vary regularization λ, plot error vs λ

### Exit

- [ ] Inverse filter consistently improves known synthetic distortion
- [ ] Condition number of Fredholm matrix computed and documented
- [ ] Regularization parameter chosen with justification (not arbitrary)
- [ ] Explain the white input spectrum assumption and its effect

---

## 6.3 Stage C — Wiener Deconvolution Equalizer (Week 21–22)

### Learn — every detail

**What remains after Volterra compensation:**
- Volterra undoes the nonlinearity (f(·) in Wiener-Hammerstein model)
- But the linear filter H₁(ω) still distorts frequency response: gain varies with frequency, phase shifts
- Wiener deconvolution inverts H₁(ω) to flatten the frequency response

**Wiener deconvolution formula:**
- `W(ω) = H₁*(ω) / (|H₁(ω)|² + λ)`
- H₁*(ω) = conjugate of estimated linear transfer function
- λ = σ²_noise / σ²_signal = noise-to-signal ratio (regularization)
- When |H₁(ω)|² >> λ: `W(ω) ≈ 1/H₁(ω)` — perfect inversion
- When |H₁(ω)|² << λ: `W(ω) ≈ 0` — suppresses frequencies where H₁ was poorly estimated (prevents noise blowup)

**Estimating λ:**
- σ²_signal: variance of the Volterra-corrected signal
- σ²_noise: estimated from high-frequency residual after Volterra correction (assuming signal energy rolls off at high freq)
- Alternative: estimate from bispectral residual
- If SNR < 20 dB: clamp λ to minimum 0.01 (PRD risk mitigation)

**Converting to FIR taps:**
- Evaluate W(ω) at 64 uniformly spaced frequency bins (0 to π)
- Take 64-point IFFT → get 64 time-domain tap coefficients w[0..63]
- Apply as FIR: `z[n] = Σ w[k] · x_volt[n-k]` for k=0..63

**Integration with EKF:**
- The 64 linear taps are part of the EKF's 80-dimensional state vector (first 64 elements)
- As EKF tracks drift, it automatically updates these taps
- No separate adaptive filter loop needed — the EKF IS the adaptation

### Build

- Create synthetic channel: `H₁(ω)` with bandpass shape (e.g., flat 100–2000 Hz, rolls off elsewhere)
- Filter white noise through H₁ → add noise → this is your "sensor output after Volterra"
- Estimate H₁(ω) from output power spectrum (assume input is white)
- Compute W(ω) with multiple λ values — plot each
- IFFT to get 64-tap FIR
- Apply to distorted signal — verify output spectrum is flat
- Plot: output spectrum with λ=0 (noise blowup) vs λ=0.01 (clean but slightly over-smoothed)

### Exit

- [ ] Output spectrum flattens as expected after Wiener equalization
- [ ] No noise blowup (regularization working)
- [ ] Explain why λ prevents noise amplification at poorly-estimated frequencies
- [ ] Show what happens when λ is too large (over-smoothing, signal loss)
- [ ] 64-tap IFFT correctly produces time-domain filter

---

## 6.4 Stage D — Extended Kalman Filter + CUSUM (Week 22–23)

### Learn — every detail

**State-space representation:**
- State equation: `x_{k+1} = f(x_k) + w_k` (w_k ~ process noise, N(0,Q))
- Measurement equation: `z_k = h(x_k) + v_k` (v_k ~ measurement noise, N(0,R))
- x_k: state vector (what you're estimating — Volterra coefficients)
- z_k: measurement (observed residual error)
- f(·): state transition (for random walk drift: f(x) = x)
- h(·): measurement function (maps coefficients to predicted residual)

**Standard Kalman Filter (linear case) — learn this first:**
- Predict: `x̂_{k|k-1} = F·x̂_{k-1}, P_{k|k-1} = F·P·F^T + Q`
- Update: `K = P·H^T·(H·P·H^T + R)^{-1}` (Kalman gain)
- `x̂_{k|k} = x̂_{k|k-1} + K·(z_k - H·x̂_{k|k-1})` (state update)
- `P_{k|k} = (I - K·H)·P_{k|k-1}` (covariance update)
- Innovation: `ν_k = z_k - H·x̂_{k|k-1}` — the "surprise" in each measurement

**Extended Kalman Filter (nonlinear case):**
- Same as KF but linearize at each step:
- F → Jacobian of f(x) at current estimate
- H → Jacobian of h(x) at current estimate
- These Jacobians must be computed analytically or numerically (finite differences)
- EKF is only locally optimal — can diverge if linearization is poor

**For SPECTER specifically:**
- State vector x (80 dimensions): [w₀, w₁, ..., w₆₃, h₂_coeff₀, ..., h₂_coeff₁₅]
- First 64 elements: Wiener equalizer FIR taps
- Last 16 elements: dominant 2nd-order Volterra kernel coefficients
- State transition: `x_{k+1} = x_k + w_k` (random walk — drift model)
- Q matrix: diagonal, ~10⁻⁶ per element per second (expected drift rate)
- R matrix: measurement noise variance, estimated online from bispectral residuals
- Measurement function h(x): maps current coefficients to predicted correction residual — this Jacobian is where the complexity is

**Tuning Q and R — the art of Kalman filtering:**
- Q too small: EKF trusts its own prediction too much — slow to track drift
- Q too large: EKF is too reactive — output is noisy
- R too small: EKF trusts measurements too much — overfits to noise
- R too large: EKF ignores measurements — doesn't adapt
- Start with Q = 1e-6·I, R estimated from data, then tune experimentally
- Innovation sequence analysis: if innovations are white (uncorrelated), Q and R are well-tuned

**CUSUM (Cumulative Sum) change detector:**
- Monitors the innovation sequence for structural changes
- Running sum: `S_k = max(0, S_{k-1} + |ε_k| - ε̄ - slack)`
- ε̄: mean innovation magnitude during normal operation
- slack: tolerance parameter — prevents false triggers from noise
- Threshold H: typically 5×σ_ε — if S_k > H, trigger re-bootstrap
- Reset S_k = 0 after trigger
- What it detects: sudden sensor changes (thermal shock, mechanical impact) that exceed the EKF's drift model

**Computational budget — critical for real-time:**
- 80×80 matrix operations: ~512K float64 multiplies for covariance update
- At 100 Hz: ~51M operations/second
- On Pi 5 with NumPy: achievable but tight. With Eigen C++: comfortable.
- Practical fallback: run EKF at 10 Hz, compensation at full sample rate with frozen coefficients between updates — demo difference is invisible

### Build — staged approach

**Stage D.1: Toy KF (4-dimensional)**
- Track: offset, gain, quadratic coefficent, noise level
- Synthetic data: inject slow drift in these 4 parameters
- Implement predict/update loop in Python
- Plot: estimated parameters vs true parameters over time
- Verify convergence

**Stage D.2: Scale to 80-dimensional EKF**
- Expand state vector to full 64 + 16 dimensions
- Use `scipy.linalg.solve` for Kalman gain (avoid explicit inversion)
- Profile: measure EKF update time in milliseconds
- If > 10 ms: reduce state dimension or move to C++ with Eigen

**Stage D.3: CUSUM integration**
- Compute innovation sequence from EKF
- Implement CUSUM accumulator
- Inject a sudden parameter change into synthetic data
- Verify: CUSUM fires within 2–5 seconds of change
- Verify: CUSUM does NOT fire during normal drift

### Exit

- [ ] EKF tracks slow drift on synthetic data (plots showing convergence)
- [ ] CUSUM fires on injected change event within 5 seconds
- [ ] CUSUM does NOT false-trigger during 5 minutes of normal operation
- [ ] EKF update time profiled — document actual ms/update on Pi 5
- [ ] Explain Q and R tuning in your own words — what each controls

---

## 6.5 Stage E — Cross-Modal Physics Consistency (Week 23–24)

### Learn — every detail

**The physics anchor — Ohm's Law as a free reference:**
- You have two sensors: voltage (V) and current (I)
- They're connected through a known fixed resistance R_load
- Physics demands: `V = I × R_load` at all times
- Any deviation `ε = ||V_corrected - I_corrected × R_load||²` means at least one sensor is wrong
- This gives you a constraint — a "free reference" — without any calibration equipment

**Joint optimization:**
- `minimize J(θ_v, θ_i) = α·J_v(θ_v) + β·J_i(θ_i) + γ·J_cross(θ_v, θ_i)`
- J_v, J_i: individual sensor calibration costs (bispectral residuals)
- J_cross: `||V_corrected(θ_v) − I_corrected(θ_i) × R_load||²`
- α, β, γ: weights. Start with α=β=1, γ=0.5, then tune.

**Alternating coordinate descent:**
- Step 1: fix θ_i, optimize θ_v to minimize J
- Step 2: fix θ_v, optimize θ_i to minimize J
- Repeat until convergence (typically 3–5 iterations)
- Converges to global optimum because J_cross is convex in each parameter set individually

**R_load requirements:**
- Must be precisely known (measure with multimeter to 0.1% if possible)
- Must be stable (USE a precision resistor, NOT a potentiometer)
- Value should be large enough for good INA219 SNR: larger R_load → larger voltage drop → better measurement

### Build

- Run both sensors simultaneously with known R_load
- Apply individual Volterra + Wiener correction to each sensor
- Compute cross-modal error: `ε = ||V_corr - I_corr × R_load||²`
- Implement alternating coordinate descent (optimize V coefficients, then I coefficients)
- Compare accuracy: with and without cross-modal term
- Show: both sensors improve simultaneously when γ > 0 (the wow factor)

### Exit

- [ ] Cross-modal error decreases after joint optimization
- [ ] Both sensors show improved accuracy with cross-modal enabled vs disabled
- [ ] R_load value measured precisely and documented
- [ ] Explain why two sensors constrain each other through physics (in your own words)

---

# PHASE 7 — Full Integration and Production Hardening (Week 25–28)

---

## 7.1 State Machine Implementation (Days 1–5)

**Five states from the PRD:**

| State | Entry Condition | Activity | Exit Condition |
|---|---|---|---|
| COLD_BOOT | Power on | Verify sensor comms, fill ring buffer | Ring buffer ≥ 512 samples |
| BOOTSTRAP | Ring buffer ready OR re-bootstrap triggered | Bispectrum accumulation, kernel extraction, Wiener tap computation | All coefficients computed successfully |
| COMPENSATING | Bootstrap complete | Full pipeline active: Volterra + Wiener + EKF + CUSUM + cross-modal | CUSUM fires OR sensor disconnected |
| RE_BOOTSTRAP | CUSUM threshold exceeded | Background re-identification on fresh data while last-known-good filter continues | New coefficients ready and validated |
| SENSOR_SWAP | New sensor detected | Full cold bootstrap on new sensor. Zero code change. | Bootstrap complete |

**Implementation pattern:**
```python
class SystemState(Enum):
    COLD_BOOT = "COLD_BOOT"
    BOOTSTRAP = "BOOTSTRAP"
    COMPENSATING = "COMPENSATING"
    RE_BOOTSTRAP = "RE_BOOTSTRAP"
    SENSOR_SWAP = "SENSOR_SWAP"

class StateMachine:
    def __init__(self):
        self.state = SystemState.COLD_BOOT
        self.transitions = {
            SystemState.COLD_BOOT: self._handle_cold_boot,
            SystemState.BOOTSTRAP: self._handle_bootstrap,
            # ...
        }
    def tick(self, context):
        handler = self.transitions[self.state]
        next_state = handler(context)
        if next_state != self.state:
            log.info(f"State transition: {self.state} -> {next_state}")
            self.state = next_state
```

**SENSOR_SWAP detection — PRD gap you must solve:**
- Method 1: monitor bispectrum structure — if the estimated H₁ and H₂ differ drastically from current model, declare new sensor
- Method 2: monitor raw signal statistics — if mean, variance, or range jump beyond N×σ thresholds, trigger swap
- Method 3: physical detection — monitor ADC channel for open-circuit/short-circuit (sensor disconnected/reconnected)
- Recommended: combine Method 2 (stats change detection) + Method 3 (hardware detection)

**Double-buffered coefficient swap:**
- Two coefficient buffers: `active` and `pending`
- Bootstrap writes to `pending`
- Compensation reads from `active`
- When bootstrap succeeds: atomic pointer/index swap — `active, pending = pending, active`
- No lock needed if swap is a single atomic write (index variable)
- Never partially update coefficients — always swap the entire set at once

---

## 7.2 Last-Known-Good Rollback (Days 5–7)

- Before each coefficient swap, save current `active` coefficients to a backup
- If new coefficients cause error increase > 50%: roll back to backup within 1 second
- Metric: cross-modal error ε and calibration confidence score
- Persistence: save to JSON file every successful bootstrap for resume-after-reboot

---

## 7.3 Health Monitoring and Watchdogs (Days 7–10)

**Software watchdog per process:**
- Each worker process sends a heartbeat timestamp to a shared memory location every 1 second
- Supervisor checks: if `now - last_heartbeat > 5 seconds` → process is dead or hung → restart it

**System health metrics to monitor continuously:**
- CPU temperature: `/sys/class/thermal/thermal_zone0/temp` — throttle computation if > 80°C
- Memory usage: `psutil.virtual_memory().percent` — warn if > 90%
- USB serial status: monitor for disconnect/reconnect
- Sample rate actual vs expected: count samples per second, alert if < 95% of target
- Queue depth: if inter-process queues fill up → backpressure detected → log warning

---

## 7.4 ESP32-S3 Dashboard (Days 10–16)

**WiFi AP mode setup:**
- ESP32-S3 creates its own access point: SSID = "SPECTER-DEMO"
- Fixed IP: 192.168.4.1 (no DHCP complications)
- No internet required — judges connect to a local network

**WebSocket server (MicroPython or Arduino C++):**
- Receives telemetry JSON from Pi over UART at 10 Hz
- Broadcasts to all connected WebSocket clients
- Handle up to 8 simultaneous connections
- Libraries: if Arduino C++: `ESPAsyncWebServer` + `AsyncWebSocket`; if MicroPython: `uwebsockets`

**Dashboard HTML/JS (served from flash):**
- Single HTML file with embedded JS and CSS
- Chart.js library for live graphs (include minified JS in flash)
- Graphs to display:
  - Raw vs corrected voltage (time series, last 5 seconds)
  - Raw vs corrected current (time series, last 5 seconds)
  - Bispectrum heatmap (2D color grid, updated per bootstrap)
  - EKF state norm over time
  - CUSUM accumulator value
  - Cross-modal consistency error
  - Calibration confidence score (0–100, color-coded)
  - System state indicator (BOOTSTRAP / COMPENSATING / RE_BOOTSTRAP)
- Mobile-responsive: use viewport meta tag, flexible grid layout
- Update rate: 10 fps via WebSocket messages

**OLED display (128×64 or 128×128):**
- Show: confidence score, CUSUM value, system state, QR code
- QR code: encode `http://192.168.4.1` — printed on hardware AND displayed on OLED
- Libraries: `Adafruit_SSD1306` or `U8g2`

**LED status strip (optional but high demo impact):**
- Solid green: calibrated and stable (COMPENSATING, confidence > 85)
- Slow pulse green: bootstrapping
- Fast amber pulse: drift detected (CUSUM rising)
- Red: change event / re-bootstrap in progress
- Use NeoPixel/WS2812B strip with `Adafruit_NeoPixel` library

---

## 7.5 Forensics Report Generation (Days 16–18)

**Report contents (generated after each bootstrap):**
1. Sensor nonlinear transfer curve: f(x) reconstructed from Volterra kernels, plotted as deviation-from-linear
2. Frequency response: |H₁(ω)| and ∠H₁(ω) vs frequency, compared to ideal flat
3. Noise floor: PSD of residual after full correction
4. Projected drift rate: from EKF Q matrix — estimated parameter change per hour per °C
5. Calibration confidence score: 0–100, derived from bispectral residual + Wiener residual + cross-modal error

**Output formats:**
- JSON (for ESP32 dashboard display)
- Human-readable text summary (for Pi terminal)
- Optional PDF (using `reportlab` or `matplotlib` figure saves)

---

## 7.6 Integration Checklist (Days 18–20)

- [ ] Frame format versioned (header byte includes version number)
- [ ] All tunables in a single `config.yaml` file (not hardcoded constants)
- [ ] Persistent logs with unique run IDs (UUID per boot)
- [ ] Deterministic startup order: Teensy → Pi acquisition → Pi DSP → ESP32
- [ ] Graceful shutdown: Ctrl-C → stop all processes → flush logs → save last-known-good state
- [ ] Error state recovery: if any module fails, system degrades gracefully (continues with last-known-good)

---

## EXIT CRITERIA — Phase 7

- [ ] 60-minute soak test passes without crash or memory leak
- [ ] Re-bootstrap succeeds while compensation continues uninterrupted
- [ ] Dashboard remains responsive during thermal stress test
- [ ] State machine transitions correctly for all 5 states
- [ ] Forensics report generates correctly after each bootstrap
- [ ] SENSOR_SWAP detection works for at least 2 different sensor types

---

# PHASE 8 — Validation, Benchmarks, and Resume Packaging (Week 29–32)

---

## 8.1 Accuracy Validation (Days 1–5)

**Test protocol:**
1. Set up reference multimeter reading same signal as SPECTER sensors
2. Apply thermal stress (hair dryer / ice pack) for 5 minutes
3. Record: raw sensor output, SPECTER-corrected output, multimeter reference
4. Compute: error = |sensor_output - multimeter_reference|
5. Repeat 10 times for statistical confidence

**Metrics to report:**
- Raw error: mean ± std under stress (expect ±18–25% per PRD)
- Corrected error: mean ± std (target ±0.6–1.2%)
- Improvement factor: raw_error / corrected_error
- Per-sensor and cross-modal results

**Be honest:** report the ACTUAL measured numbers, not theoretical. If corrected error is ±3% instead of ±1%, that's still excellent — just report it accurately.

---

## 8.2 Latency Benchmarking (Days 5–7)

**End-to-end latency measurement:**
- Teensy: set GPIO pin HIGH when sample is acquired
- Pi: set GPIO pin HIGH when corrected output is available
- Measure time between the two rising edges with oscilloscope
- Record 10,000 measurements

**Metrics:**
- p50 (median), p95, p99, max latency
- Target: < 15 ms end-to-end (PRD claim)
- Latency histogram plot

**Component-level profiling:**
- Teensy acquisition ISR execution time (ARM_DWT_CYCCNT)
- USB serial transfer time
- Volterra compensation per frame
- Wiener filter per frame
- EKF update per cycle
- Telemetry dispatch

---

## 8.3 Bootstrap Performance (Days 7–9)

- Run 20 cold-start bootstraps
- Record time to reach COMPENSATING state for each
- Report: mean, min, max, std
- Target: < 8 seconds (PRD claim)
- Plot: bootstrap time vs number of frames (M)

---

## 8.4 Robustness Testing (Days 9–12)

**Stress test scenarios:**
- Thermal stress: sudden temperature change (hair dryer then ice pack)
- Sensor swap: hot-plug a different sensor type
- USB disconnect: pull and reconnect Teensy USB cable
- Power glitch: briefly interrupt sensor power
- For each: document behavior, recovery time, any data loss

**Fault injection:**
- Corrupt 5% of USB serial packets — verify checksum catches them
- Inject NaN into coefficient vector — verify system rolls back
- Kill EKF worker process — verify supervisor restarts it

---

## 8.5 Documentation and README (Days 12–16)

**README.md requirements:**
- Architecture diagram (draw it — Mermaid, draw.io, or hand-drawn)
- Hardware photo with labeled components
- Getting started: exact steps to reproduce from bare hardware
- Every claim backed by a measurement (link to plots)
- Known limitations section — be honest

**Technical report (2–3 pages):**
- Problem statement
- Method summary (pipeline diagram)
- Key assumptions (Gaussianity, stationarity, ergodicity, white input spectrum)
- Results: accuracy tables, latency histograms, bootstrap time distributions
- Limitations: what conditions cause degraded performance
- Future work

**Demo video:**
- 5–7 minute walkthrough matching the 5-act demo plan from PRD
- Screen recording of dashboard + camera on hardware
- Narrated or subtitled

---

## EXIT CRITERIA — Phase 8

- [ ] Every claim in README has a corresponding measured data point
- [ ] Accuracy plots saved as PNGs with proper axes, labels, legend
- [ ] Latency histogram with p50/p95/p99 computed and labeled
- [ ] Bootstrap time distribution across ≥ 20 runs
- [ ] Demo video recorded and uploaded
- [ ] You can explain failure modes and mitigations without handwaving
- [ ] README is comprehensible to someone who has never seen the project

---

# SUPPLEMENTARY SECTIONS

---

## Daily Routine (Use This for Momentum)

1. **30 min theory** — one concept only (not five)
2. **90 min coding** — implement one tiny thing tied to that concept
3. **30 min validation** — compare output to expected behavior. Plot it.
4. **15 min notes** — write "what worked, what failed, why"
5. **15 min cleanup** — commit code, update todo

This compounding loop is how beginners become dangerous engineers.

---

## Learning Resources (Pick 1–2 per category, not 10)

**C/C++ Embedded:**
- K&R "The C Programming Language" (chapters 1–6 minimum)
- Teensy official documentation and examples
- ARM Cortex-M7 Technical Reference Manual (for register details)
- CMSIS-DSP library documentation

**DSP:**
- Oppenheim & Willsky "Signals and Systems" (chapters 1–5, 10)
- Oppenheim & Schafer "Discrete-Time Signal Processing" (chapters 2–5, 8–10)
- Smith "The Scientist and Engineer's Guide to DSP" (free online — practical, code-heavy)
- NumPy/SciPy signal processing tutorials

**Estimation Theory:**
- Dan Simon "Optimal State Estimation" (chapters 1–7 for EKF)
- Roger Labbe "Kalman and Bayesian Filters in Python" (free online — excellent)
- Any practical EKF implementation walkthrough with code (not just math)

**Higher-Order Statistics:**
- Nikias & Petropulu "Higher-Order Spectra Analysis" (the textbook on bispectrum)
- Mendel "Tutorial on Higher-Order Statistics" (IEEE Proceedings 1991 — classic)

**Python Systems:**
- Python `multiprocessing` official documentation
- Real Python tutorials on concurrency and parallelism

Rule: prefer resources with code and plots over purely abstract derivations.

---

## Risk Map (What Usually Breaks Beginners)

| Risk | Fix |
|---|---|
| Trying full system too early | Build toy models first, scale after toy converges |
| No synthetic test data | ALWAYS validate on known signals before real sensors |
| Skipping instrumentation | Add timing and error metrics from day 1 |
| Mixing algorithm bugs with hardware bugs | Isolate layers, verify one at a time |
| Overclaiming performance | Publish measured values only, never theoretical promises |
| Debugging by guessing | Reproduce → Isolate → Measure → Fix → Verify |
| Perfectionism paralysis | Build ugly first, optimize second |
| Not committing to Git regularly | Commit after every working change, branch for experiments |

---

## Capstone Build Order (Exact Sequence)

Follow this order. If any step is unstable, do NOT continue.

1. Teensy acquisition stable and timestamped (Phase 4)
2. Pi receiver stable with checksum and logging (Phase 4–5)
3. FIR-only correction path working (Phase 3 mini-project → Phase 5 pipeline)
4. Bispectrum estimation validated on synthetic data (Phase 6.1)
5. Volterra compensation validated on synthetic data (Phase 6.2)
6. Volterra compensation on real sensor data (Phase 6.2 → real hardware)
7. Wiener equalizer integrated and validated (Phase 6.3)
8. EKF toy model converging (Phase 6.4, start with 4-dim)
9. EKF scaled to full 80-dim (or reduced dim) — profiled on Pi (Phase 6.4)
10. CUSUM trigger and re-bootstrap plumbing (Phase 6.4)
11. Cross-modal term integrated — both sensors improving together (Phase 6.5)
12. ESP32 dashboard live with all graphs (Phase 7.4)
13. Forensics report generation (Phase 7.5)
14. State machine with all 5 states working (Phase 7.1)
15. Full 60-minute soak test (Phase 7 exit)
16. Accuracy and latency benchmarks (Phase 8)
17. README, report, and demo video (Phase 8)

---

## Definition of Done (True Completion)

You are "done" only when ALL of these are true:

- [ ] End-to-end system runs for 1 hour without crash
- [ ] Corrected output statistically outperforms raw output under thermal stress
- [ ] Re-bootstrap works automatically on induced change event
- [ ] Sensor swap flow works with at least 2 sensor types
- [ ] Cross-modal term demonstrably improves both sensors simultaneously
- [ ] All core claims backed by saved plots and measured data (not theory)
- [ ] You can explain each equation in your own words and map it to code
- [ ] README is comprehensible to someone who has never seen the project
- [ ] Demo video recorded

---

## Personal Progress Tracker

Use this table in your notes weekly:

| Week | Phase | What I Built | What Failed | What I Fixed | Hours | Next Step |
|---|---|---|---|---|---|---|
| W1 | P1 | | | | | |
| W2 | P1 | | | | | |
| W3 | P1 | | | | | |
| W4 | P2 | | | | | |
| ... | ... | | | | | |

Keep this brutally honest. Every hard bug you solve becomes interview gold if documented clearly.

---

## Final Advice

- Build ugly first, then optimize.
- Do not chase perfect architecture before first successful run.
- Preserve a working baseline branch always (`git branch baseline` before risky changes).
- Test on synthetic BEFORE real hardware — always.
- When stuck for > 2 hours: step back, write down what you know, what you don't, and what experiment would disambiguate.
- Every hard bug you solve becomes interview gold if documented clearly.

You do not need to be a genius to build this. You need consistency, isolation, measurement, and patience.

---

*File: `docs/roadmap.md`*
*Scope: complete beginner to full SPECTER self-build capability*
*Every topic. Every concept. No hand-waving.*
