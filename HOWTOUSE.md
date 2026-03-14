# HOWTOUSE.md — How to Use AI to Build SPECTER

**The file you read before starting any AI session.**
Every scenario you'll face, every file combination you'll need, every prompt formula that works.

---

## Quick Reference — File Purpose Map

| File | What It Contains | When AI Needs It |
|---|---|---|
| `AI_RULES.md` | Hard constraints, code style rules, what's forbidden | **Every single session** |
| `TASKS.md` | 18 ordered build chunks with exact specs | Every build session |
| `docs/ARCHITECTURE.md` | System design, process layout, state machine, IPC | When building any Pi module or state logic |
| `docs/DATA_MODELS.md` | Every data structure, packet format, shared memory layout | When touching serial, shared memory, EKF state, telemetry |
| `docs/TECH_STACK.md` | Library versions, platformio configs, hardware specs | When starting firmware, setting up deps, or debugging library issues |
| `docs/PROJECT_STRUCTURE.md` | Every file and its responsibility | When creating new files or understanding what goes where |
| `config/specter_config.yaml` | All tunable parameters | When implementing any algorithm that has a tunable (lambda, EKF Q, etc.) |
| `docs/SPECTER_PRD updated.md` | Full product requirements, theory background, demo plan | When you need to understand WHY something exists |
| `docs/roadmap.md` | Learning roadmap with every concept to study | Learning sessions only — NOT for build sessions |
| `prompt/AI_Learning_Coach_Prompt (1).md` | Learning coach system prompt | Learning sessions with a study AI only |
| `specter/*.py` | Current code stubs/implementations | When implementing, debugging, or extending existing code |
| `firmware/teensy/*.ini` | PlatformIO config for Teensy | Teensy build sessions |
| `firmware/esp32/*.ini` | PlatformIO config for ESP32 | ESP32 build sessions |
| `tests/conftest.py` | Shared test fixtures (synthetic signals) | When writing or running tests |

---

## The Core Prompt Formula

Every AI build session prompt follows this structure:

```
Read these files first, in this order:
1. [LIST OF FILES]

Then: [YOUR TASK]

Do NOT do anything beyond what I asked.
Do NOT modify AI_RULES.md.
```

The files come first. Always. The AI needs context before it can write correct code.

---

# SCENARIO 1: Starting the Project From Scratch

**Use this when:** You are starting fresh on a new machine or a new AI session with zero previous context.

**Files to include (paste or attach):**
1. `AI_RULES.md`
2. `docs/ARCHITECTURE.md`
3. `docs/PROJECT_STRUCTURE.md`
4. `docs/TECH_STACK.md`
5. `TASKS.md`

**Prompt:**
```
Read these 5 files in order:
1. AI_RULES.md
2. docs/ARCHITECTURE.md
3. docs/PROJECT_STRUCTURE.md
4. docs/TECH_STACK.md
5. TASKS.md

You are now an embedded DSP engineer helping me build the SPECTER project.
Your job is to implement task chunks from TASKS.md in strict order.

The first unchecked task chunk is C-00. Implement it exactly as specified.
Follow AI_RULES.md at all times — especially the language boundary rules.
After completing C-00, show me what you created and stop.
```

---

# SCENARIO 2: Implementing a Specific Task Chunk

**Use this when:** You want the AI to implement one specific chunk (e.g., C-07 bispectrum estimation).

**Files to include:**
1. `AI_RULES.md` — mandatory always
2. `TASKS.md` — so it knows the exact requirements
3. `docs/ARCHITECTURE.md` — so it understands system context
4. `docs/DATA_MODELS.md` — if the chunk involves data structures or serial packets
5. The CURRENT version of the target source file (e.g., `specter/bootstrap.py`) — so it knows what already exists
6. `tests/conftest.py` — if the chunk includes writing tests

**Prompt:**
```
Read these files first:
1. AI_RULES.md
2. TASKS.md
3. docs/ARCHITECTURE.md
4. docs/DATA_MODELS.md
5. specter/bootstrap.py (current content below)

[PASTE FILE CONTENT HERE]

Implement TASKS.md chunk C-07 (Bispectrum Estimation) exactly as specified.

Rules:
- Follow AI_RULES.md section 3.3 (pure NumPy, no Python loops in hot path)
- Only implement what C-07 specifies — do not implement C-08 yet
- Write the test file tests/synthetic/test_bispectrum.py as specified
- Show me the implementation and the test, then stop
```

**What changes per chunk:**
- Change the file name in step 5 to the actual module being built
- Change "C-07" to whichever chunk you're on
- Add `docs/DATA_MODELS.md` for chunks involving serial, shared memory, or structured data
- Add `config/specter_config.yaml` for chunks involving tunable parameters

---

# SCENARIO 3: Debugging a Failing Test

**Use this when:** A test is failing and you need the AI to find and fix the bug.

**Files to include:**
1. `AI_RULES.md`
2. The failing test file (e.g., `tests/synthetic/test_bispectrum.py`)
3. The module under test (e.g., `specter/bootstrap.py`)
4. `docs/DATA_MODELS.md` if the bug involves data formats
5. The EXACT error output from `pytest`

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. tests/synthetic/test_bispectrum.py (content below)
3. specter/bootstrap.py (content below)

[PASTE BOTH FILE CONTENTS]

Here is the exact error I'm getting:
[PASTE FULL pytest OUTPUT HERE — include the stack trace]

Find the bug. Explain what is wrong and why before writing any fix.
Then write the minimal fix. Do NOT change the test — the test is correct.
Do NOT change the function signature.
```

**Key rule:** Always paste the FULL error output including the stack trace. Never summarize it.

---

# SCENARIO 4: Debugging a Hardware Issue (Teensy Firmware)

**Use this when:** Teensy firmware compiles but behaves wrongly — wrong sample rate, bad data, crashes.

**Files to include:**
1. `AI_RULES.md`
2. `docs/ARCHITECTURE.md` (Teensy section 6)
3. `docs/TECH_STACK.md` (Teensy hardware specs)
4. `docs/DATA_MODELS.md` (serial packet format)
5. The specific C++ files with the bug (e.g., `firmware/teensy/src/acquisition.cpp`)
6. `firmware/teensy/src/acquisition.h`
7. Description of the observed behavior vs expected behavior

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. docs/ARCHITECTURE.md (section 6, Teensy firmware architecture)
3. docs/TECH_STACK.md
4. docs/DATA_MODELS.md (section 1, SampleFrame struct)
5. firmware/teensy/src/acquisition.h (content below)
6. firmware/teensy/src/acquisition.cpp (content below)

[PASTE FILE CONTENTS]

PROBLEM:
Expected: GPIO toggles at 3200 Hz with < 100 ns jitter
Observed: GPIO toggles at 3200 Hz but jitter is 800 ns

Oscilloscope readings:
- Period: 312.5 μs ± 0.8 μs (should be ± 0.1 μs)

What could cause high jitter in the IntervalTimer ISR despite correct average rate?
Diagnose, then propose a fix. Follow AI_RULES.md section 3.6 (ISR rules).
```

---

# SCENARIO 5: Debugging a Pi Inter-Process Issue

**Use this when:** Processes are miscommunicating, shared memory is corrupt, or a process is crashing.

**Files to include:**
1. `AI_RULES.md`
2. `docs/ARCHITECTURE.md` (section 2, process layout + section 9, shared memory)
3. `docs/DATA_MODELS.md` (sections 2–6, shared memory layouts)
4. `specter/shared_memory_layout.py`
5. The two processes involved (both files)
6. The log output showing the error

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. docs/ARCHITECTURE.md
3. docs/DATA_MODELS.md
4. specter/shared_memory_layout.py (content below)
5. specter/acquire.py (content below)
6. specter/compensate.py (content below)

[PASTE ALL FILE CONTENTS]

PROBLEM: compensate.py is reading garbage values from the ring buffer.
acquire.py appears to be writing correctly (I verified timestamps are valid).

Here is the log output showing the issue:
[PASTE LOG OUTPUT]

Diagnose the root cause. Is it a write pointer issue, a dtype mismatch, or a race condition?
Show the exact fix. Follow AI_RULES.md section 3.4 (shared memory rules).
```

---

# SCENARIO 6: Implementing an Algorithm From Theory

**Use this when:** You want the AI to implement one of the DSP algorithms (bispectrum, Volterra, EKF, etc.) and you're not sure the AI knows the theory.

**Files to include:**
1. `AI_RULES.md`
2. `docs/ARCHITECTURE.md` (the DSP pipeline section 3)
3. `docs/SPECTER_PRD updated.md` (sections 4 and 7 — theory + methodology)
4. `config/specter_config.yaml` — so it uses the right parameters
5. The target stub file

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. docs/ARCHITECTURE.md (section 3, DSP pipeline)
3. docs/SPECTER_PRD updated.md (sections 4.1–4.3 and 7.1–7.3)
4. config/specter_config.yaml (content below)
5. specter/bootstrap.py (current stub below)

[PASTE CONTENTS]

Implement the `estimate_bispectrum()` function in specter/bootstrap.py.

Theory reference from the PRD (section 4.1):
- B̂(k1,k2) = X(k1)·X(k2)·X*(k1+k2)
- Average over M frames
- Principal domain only (see PRD section 4.1 for the symmetry bounds)

Implementation requirements from TASKS.md C-07:
- Input: (M, fft_size) array of signal frames
- Output: (fft_size//2, fft_size//2) complex128 array
- Must use np.fft.rfft (not np.fft.fft)
- Hanning window before FFT
- No Python loops — vectorized NumPy only

Show the function implementation and explain each step in comments.
```

---

# SCENARIO 7: Adding a New Feature Not in TASKS.md

**Use this when:** You want to add something the task list doesn't cover (e.g., a new visualization, a logging improvement, a safety check).

**Files to include:**
1. `AI_RULES.md`
2. `docs/PROJECT_STRUCTURE.md` — so it puts files in the right place
3. `docs/ARCHITECTURE.md` — so it doesn't break existing boundaries
4. The file(s) that will be modified

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. docs/PROJECT_STRUCTURE.md
3. docs/ARCHITECTURE.md
4. specter/telemetry.py (content below)

[PASTE CONTENT]

Add a feature: log a warning to the logging system whenever the cross-modal error ε
exceeds 0.05 (i.e., more than 5% deviation from Ohm's Law).

Constraints:
- Follow AI_RULES.md section 4 (code style — use logging, not print)
- Do NOT add any new dependencies
- Do NOT modify any other file
- The warning log should include the exact ε value and the current system state
```

---

# SCENARIO 8: Profiling and Performance Optimization

**Use this when:** A stage is too slow and you need the AI to find the bottleneck and fix it.

**Files to include:**
1. `AI_RULES.md`
2. `docs/TECH_STACK.md` (Pi hardware specs — so the AI knows what's fast/slow)
3. The slow module (e.g., `specter/compensate.py`)
4. Profiling output from `python scripts/profile_pi.py` or `cProfile`

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. docs/TECH_STACK.md (Raspberry Pi 5 section — hardware specs)
3. specter/compensate.py (content below)

[PASTE CONTENT]

Here is the profiling output from `python -m cProfile -s cumtime specter/compensate.py`:
[PASTE CPROFILE OUTPUT]

The total compensation time is 12 ms per 512-sample frame.
Target from TASKS.md C-12: < 5 ms per frame.

Identify the bottleneck. The quadratic Volterra term is likely the issue (O(L²) per sample).
Propose a vectorized NumPy fix — zero Python loops allowed (AI_RULES.md section 3.3).
Show before and after timing estimates.
```

---

# SCENARIO 9: ESP32 Dashboard Modification

**Use this when:** You want to change the dashboard HTML, add a new graph, or fix a WebSocket issue.

**Files to include:**
1. `AI_RULES.md`
2. `docs/DATA_MODELS.md` (section 5 — telemetry JSON schema)
3. `docs/TECH_STACK.md` (ESP32 libraries list)
4. `firmware/esp32/src/data/index.html` (current dashboard)
5. `firmware/esp32/src/wifi_ap.cpp` if WebSocket logic is involved

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. docs/DATA_MODELS.md (section 5 — telemetry JSON schema: what fields are available)
3. firmware/esp32/src/data/index.html (current dashboard below)

[PASTE FILE CONTENT]

Add a 7th graph to the dashboard: "R_load Estimation vs Time" (field `r_load_estimated` from telemetry).

Constraints:
- Single HTML file only — no external CDN, all JS inline or from SPIFFS
- Mobile-responsive, same style as existing graphs
- WebSocket data already arrives at 10 Hz — just add a new Chart.js dataset
- Do NOT change the WebSocket protocol or data schema
```

---

# SCENARIO 10: Config Tuning

**Use this when:** The algorithm isn't converging or the EKF is diverging and you need help adjusting parameters.

**Files to include:**
1. `AI_RULES.md`
2. `config/specter_config.yaml`
3. `docs/TECH_STACK.md` (the config model section — what each parameter does)
4. Log output showing the problem (EKF state norm, bispectrum convergence plots, CUSUM false fires)

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. config/specter_config.yaml (content below)
3. docs/TECH_STACK.md (config file section — parameter meanings)

[PASTE CONFIG CONTENT]

Problem: CUSUM is triggering false alarms every ~30 seconds during steady-state operation.
The innovation sequence looks like this:
[PASTE INNOVATION VALUES OR PLOT DESCRIPTION]

Current settings:
- cusum_slack_factor: 1.0
- cusum_threshold_sigma: 5.0

Diagnose: is this a Q too large (EKF over-reporting uncertainty), slack too tight, or threshold too low?
What values of these 3 parameters should I try, and why?
Give me a specific set of values to test first, with justification.
Only change config values — do NOT change any code.
```

---

# SCENARIO 11: Writing Tests for Existing Code

**Use this when:** A module is implemented but has no tests yet.

**Files to include:**
1. `AI_RULES.md`
2. `tests/conftest.py` — so it uses the existing fixtures
3. The module to test (e.g., `specter/crossmodal.py`)
4. `TASKS.md` (find the test spec for that chunk)

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. tests/conftest.py (fixtures available — content below)
3. specter/crossmodal.py (the module to test — content below)
4. TASKS.md (section C-11 — the test requirements)

[PASTE ALL CONTENTS]

Write tests/synthetic/test_crossmodal.py as specified in TASKS.md C-11.

Requirements from C-11:
- Use `synthetic_sensor_signal` fixture from conftest.py
- Add 3% gain error to V, -2% to I
- Run alternating descent with γ=0.5
- Assert cross-modal error drops ≥30% after 5 iterations

Do NOT change crossmodal.py. Only write the test file.
All tests must use pytest conventions. No bare assert without message.
```

---

# SCENARIO 12: End-of-Session Handover (Starting a New AI Session)

**Use this when:** A session is ending (or the AI has lost context) and you want to resume cleanly in a fresh session.

**Files to include:**
1. `AI_RULES.md`
2. `TASKS.md` (mark current progress first — update `[x]` and `[/]` before starting new session)
3. `docs/ARCHITECTURE.md`
4. The specific files you were working on

**Prompt for new session:**
```
Read these files in order:
1. AI_RULES.md
2. TASKS.md
3. docs/ARCHITECTURE.md

[PASTE ALL CONTENTS]

Context: I am building the SPECTER project. The last task I completed was C-07.
I am now starting C-08 (Volterra kernel extraction).

Here is the current state of specter/bootstrap.py (the file I'll be modifying):
[PASTE CURRENT FILE CONTENT]

Start implementing C-08. Read the requirements from TASKS.md before writing any code.
```

---

# SCENARIO 13: Learning a Concept (Study Session — NOT a Build Session)

**Use this when:** You want to understand a SPECTER algorithm before implementing it.

**Files to include:**
1. `prompt/AI_Learning_Coach_Prompt (1).md` — the study coach prompt (SYSTEM PROMPT)
2. `docs/roadmap.md` — the learning roadmap (so it knows what level you're at)
3. `docs/SPECTER_PRD updated.md` sections 4 and 7 — PRD theory background

**How to set up the session:**
```
SYSTEM PROMPT: [paste entire content of prompt/AI_Learning_Coach_Prompt (1).md]

USER MESSAGE:
I am a 3rd-year engineering student building the SPECTER project.
I want to deeply understand bispectrum estimation before implementing it.

Reference material (from my project PRD — docs/SPECTER_PRD updated.md, section 4.1):
[PASTE SECTION 4.1 — HIGHER ORDER STATISTICS]

My current knowledge level: I understand FFT and power spectra but have never used
higher-order statistics.

Start the teaching session for: "Bispectrum and Third-Order Cumulants"
Follow the coach prompt format: Concept → Derivation → Why it works → Worked example →
Homework problem → Exit criteria.
```

**Important:** This is a STUDY session, not a BUILD session. Do NOT paste `AI_RULES.md` here — the coach prompt is the system prompt, not the coding rules.

---

# SCENARIO 14: Explaining an Existing File to Yourself

**Use this when:** You wrote code weeks ago, you're back, and you want the AI to explain what the code does before you touch it.

**Files to include:**
1. The file you want explained
2. `docs/ARCHITECTURE.md` (for system context)

**Prompt:**
```
Read these files:
1. docs/ARCHITECTURE.md
2. specter/kalman.py (content below)

[PASTE FILE CONTENT]

Explain this file to me as if you're a senior engineer doing a code review:
1. What does it do, in one sentence?
2. Walk through the SpectreEKF class — what does each method do?
3. What are the inputs and outputs of the predict/update cycle?
4. Are there any bugs, edge cases, or numerical issues I should be aware of?

Do NOT change any code. Just explain.
```

---

# SCENARIO 15: Full System Integration Check

**Use this when:** You think the full Pi pipeline is complete and you want the AI to review everything for integration issues.

**Files to include:**
1. `AI_RULES.md`
2. `docs/ARCHITECTURE.md`
3. `docs/DATA_MODELS.md`
4. All `specter/*.py` files (all 10 of them)

**Prompt:**
```
Read these files:
1. AI_RULES.md
2. docs/ARCHITECTURE.md
3. docs/DATA_MODELS.md
4. ALL specter/*.py files (contents below)

[PASTE ALL FILE CONTENTS — in this order: shared_memory_layout.py, state.py,
config_loader.py, acquire.py, compensate.py, bootstrap.py, kalman.py,
crossmodal.py, forensics.py, telemetry.py, main.py]

Perform a full integration review. Check for:
1. Shared memory name mismatches between files
2. Dtype mismatches between writers and readers
3. Missing lock/unlock pairs on shared coefficient writes
4. Any module that violates AI_RULES.md section 3 (hard rules)
5. Race conditions in the ring buffer read/write pointers
6. Any missing heartbeat calls in worker processes

List every issue found. Use the format:
FILE: [filename]
ISSUE: [description]
SEVERITY: [critical/warning/info]
FIX: [exact code change]
```

---

# SCENARIO 16: Preparing for the Demo

**Use this when:** The system is working and you want to rehearse or fix the 5-act demo.

**Files to include:**
1. `AI_RULES.md`
2. `docs/SPECTER_PRD updated.md` (section 9 — Demonstration Plan)
3. `specter/main.py` (the state machine)
4. `firmware/esp32/src/data/index.html` (the dashboard)

**Prompt:**
```
Read these files:
1. docs/SPECTER_PRD updated.md (section 9 — Demonstration Plan)
2. specter/main.py (content below)
3. firmware/esp32/src/data/index.html (content below)

[PASTE CONTENTS]

I am preparing for the 5-act demo described in PRD section 9.

For each act, tell me:
1. What the system state should be at the start of that act
2. What the dashboard should be showing
3. What could go wrong and how to recover

Then: for Act 4 (adversarial stress + autonomous recovery), what is the minimum
CUSUM threshold that will fire during an ice pack stress test but NOT fire during
normal 5°C ambient temperature variation?
Help me calculate the right config values for cusum_threshold_sigma.
```

---

# RULES FOR ALL SESSIONS

## Always Do This

- **Paste file contents explicitly** — most AI tools cannot read files from your disk. Copy-paste them into the prompt.
- **Start with `AI_RULES.md` first** — before any other file, every time.
- **Update `TASKS.md` after each chunk** — mark `[x]` only after the validation passes.
- **One chunk per session** — don't ask the AI to implement C-07 and C-08 in the same session. You lose quality fast.
- **Copy-paste actual error messages** — never say "it gives an error". Paste the full traceback.

## Never Do This

- **Never say "implement the whole project"** — the AI will hallucinate half of it. Always give a single, specific chunk.
- **Never paste AI_RULES.md without also pasting the relevant architecture doc** — the rules reference the architecture constantly.
- **Never skip the test step** — don't mark a task `[x]` until you've run the tests and they pass.
- **Never let the AI change `AI_RULES.md`** — it's a constraint document, not a negotiating document.
- **Never paste `roadmap.md` in a build session** — it's for study sessions only. It confuses build context.
- **Never paste more than 5–6 files in one prompt** — beyond that, the AI loses focus on the earlier files.

---

## Session Size Guide

| Complexity | Chunks Per Session | Files to Include |
|---|---|---|
| Quick fix (typo, renaming) | 1 fix | `AI_RULES.md` + affected file |
| Single function implementation | 1 function | `AI_RULES.md` + TASKS.md + 1–2 source files |
| Full chunk implementation (C-07 etc.) | 1 chunk | `AI_RULES.md` + TASKS.md + ARCHITECTURE + 2–3 source files |
| Debugging session | 1 bug | `AI_RULES.md` + 2 source files + full error output |
| Integration review | 0 new code | `AI_RULES.md` + ARCHITECTURE + DATA_MODELS + all `specter/*.py` |
| Study session | N/A | Coach prompt (system) + PRD section + roadmap |

---

## The 30-Second Checklist Before Every AI Session

1. **Did you update `TASKS.md` to reflect what's already done?** (mark `[x]` on completed items)
2. **Do you know which chunk you're working on?** (one chunk at a time)
3. **Have you read the chunk's validation criteria in TASKS.md?** (know what "done" means before starting)
4. **Are you pasting `AI_RULES.md` first?** (always)
5. **Do you have the actual current file content to paste?** (don't rely on the AI to "remember" old code)

---

*File: `HOWTOUSE.md`*
*Keep this file updated as you discover new workflows that work well.*
