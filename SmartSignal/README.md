# SmartSignal

**A C++ traffic light controller simulation built to mirror embedded systems design.**

SmartSignal is a 4-way intersection traffic controller that uses a finite state machine, event-driven logic, and software-timed execution. It simulates the kind of firmware you’d run on a microcontroller controlling real traffic lights—but runs on your laptop for development and testing.

---

## Features

- **Finite state machine** — Explicit phases (NS_GREEN → NS_YELLOW → ALL_RED → EW_GREEN → EW_YELLOW → ALL_RED) with configurable timing
- **Event arbitration** — Priority encoder: Crash > Siren > Manual Override > Ped Button > Timer
- **Debounced inputs** — Pedestrian buttons and emergency sensors filtered with hysteresis
- **Siren detection** — Frequency/amplitude analysis with configurable hysteresis
- **Simulated GPIO** — Memory-mapped register model for inputs and outputs
- **Deterministic timing** — Fixed 50 ms tick (configurable); no blocking, no RTOS
- **CSV logging** — Per-tick log for analysis and visualization

---

## Build & Run

```bash
mkdir build && cd build
cmake ..
cmake --build .
./smartsignal    # or smartsignal.exe on Windows
```

Logs are written to `logs/run.csv`. To visualize:

```bash
pip install matplotlib
python scripts/visualize_log.py
```

---

## Project Structure

```
SmartSignal/
├── include/smartsignal/   # Headers
│   ├── types.hpp          # ms_t, LampState, Outputs, AudioSample, DetectionResult
│   ├── events.hpp         # Event enum, EventFlags, pick_highest_priority
│   ├── config.hpp         # Config struct, file loading
│   ├── gpio.hpp           # Memory-mapped I/O simulation
│   ├── debounce.hpp       # Input debouncing
│   ├── detection.hpp      # Siren detection
│   ├── fsm.hpp            # Traffic FSM
│   ├── arbiter.hpp        # Event priority (uses events.hpp)
│   └── logger.hpp         # RAII CSV logger
├── src/                   # Implementation
├── tests/                 # Unit tests
├── docs/                  # STATE_DIAGRAM, TIMING_SPEC, ECE_CONCEPTS
├── config/default.yaml    # Tunable parameters
└── scripts/visualize_log.py
```

---

## Embedded Systems Concepts

SmartSignal is built to reflect real embedded system design. Here’s how it maps:

### 1. Superloop (Main Loop)

**Concept:** Many embedded systems use a single infinite loop instead of an RTOS. Each iteration does: read inputs → process → write outputs. Timing is driven by a fixed tick.

**In SmartSignal:** `main.cpp` runs a `while` loop at `tick_ms` (default 50 ms). Every tick: simulate sensors → detection → debounce → arbitrate events → step FSM → write outputs → log. No threads, no blocking I/O.

### 2. Memory-Mapped I/O

**Concept:** On a microcontroller, peripherals (GPIO, timers, UART) appear as registers at fixed memory addresses. You read/write them like variables.

**In SmartSignal:** `gpio.hpp` defines a `GPIO` class with `input_reg_` and `output_reg_`. `read_inputs()` / `write_inputs()` simulate reading pins; `write_outputs()` / `read_outputs()` simulate driving lamps. Same mental model as `*(volatile uint32_t*)0x40020000 = value;` on real hardware.

### 3. Finite State Machine (FSM)

**Concept:** Traffic lights are a textbook FSM: each state has defined outputs and transition conditions. Hardware FSMs are common in digital logic (ECE 120).

**In SmartSignal:** `TrafficFSM` has a `Phase` enum and `step()` / `get_outputs()`. Next-state logic and output logic are separate. Transitions occur when the timer fires and the phase duration has elapsed.

### 4. Priority Encoder / Event Arbitration

**Concept:** Multiple events can occur at once. A priority encoder picks the single highest-priority one. Common in interrupt controllers and real-time systems.

**In SmartSignal:** `arbiter.cpp` implements `pick_highest_priority()`. Given `EventFlags` (e.g., Timer and Siren both active), it returns Siren because Siren has higher priority. Same idea as hardware priority encoders in digital logic.

### 5. Debouncing

**Concept:** Mechanical buttons and sensors “bounce”—produce multiple edges in a few milliseconds. Firmware must filter this with hysteresis: require N consecutive same readings before changing state.

**In SmartSignal:** `Debouncer` needs N consecutive high (or low) readings before flipping output. Uses a simple counter, analogous to a shift-register or counter-based debounce in hardware.

### 6. Hysteresis in Detection

**Concept:** Sensors are noisy. A single threshold causes rapid on/off toggling. Hysteresis uses separate “turn on” and “turn off” thresholds (or sample counts) to avoid chattering.

**In SmartSignal:** Siren detection uses `trigger_samples` and `clear_samples`. It takes 6 consecutive high-confidence readings to set “detected,” and 10 consecutive low-confidence readings to clear it. Same idea as Schmitt triggers in analog design.

### 7. RAII (Resource Acquisition Is Initialization)

**Concept:** In C++, constructors acquire resources (file handles, locks); destructors release them. No manual `close()` needed if you use scope correctly.

**In SmartSignal:** `Logger` opens the file in `open()` and closes in the destructor. If the program exits or throws, the destructor runs and the file is closed. Matches good embedded practice: acquire in init, release in shutdown.

### 8. Deterministic Timing

**Concept:** Embedded control loops often run at a fixed rate (e.g., 100 Hz). Timestamps are in ticks or milliseconds since boot.

**In SmartSignal:** `ms_t` is milliseconds since sim start. `tick_ms` defines the loop period. Phase durations (green_ms, yellow_ms, etc.) are explicit. Behavior is deterministic and repeatable.

---

## Configuration

Edit `config/default.yaml`:

```yaml
green_ms: 5000
yellow_ms: 2000
all_red_ms: 1000
tick_ms: 50
sim_duration_ms: 30000
siren_conf_threshold: 0.75
log_filename: logs/run.csv
```

---

## Documentation

- [State Diagram](docs/STATE_DIAGRAM.md) — FSM phases and transitions
- [Timing Spec](docs/TIMING_SPEC.md) — Phase durations, tick rate, debounce
- [ECE Concepts](docs/ECE_CONCEPTS.md) — Mapping to ECE 120, ECE 220, CS 128

---

## Simulation Scenarios

- **Pedestrian button:** Simulated 3–3.5 s; debounced and raised as `PedButton` event
- **Siren:** Simulated 8–13 s; frequency/amplitude ramp; detection with hysteresis
- **Normal cycle:** Timer-driven phase transitions; logs written every tick

---

## License

MIT
