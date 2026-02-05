# SmartSignal and ECE Course Concepts

This document maps SmartSignal modules to concepts from ECE 120, ECE 220, and CS 128.

---

## ECE 120: Digital Logic Design

| Concept | Where in SmartSignal |
|---------|----------------------|
| **Finite State Machine** | `fsm.cpp` – Phase enum, next_phase(), explicit state transitions |
| **State encoding** | Phase enum is the state; outputs derived from current state (combinational output logic) |
| **Priority encoder** | `arbiter.cpp` – Multiple events in, single highest-priority event out |
| **Debouncing (sequential logic)** | `debounce.cpp` – Shift-register style: N consecutive same readings before output flips |
| **Combinational vs sequential** | next-state logic (step) vs output logic (get_outputs) separated clearly |

---

## ECE 220: Computer Systems & Programming

| Concept | Where in SmartSignal |
|---------|----------------------|
| **Memory-mapped I/O** | `gpio.hpp/cpp` – Registers (input_reg_, output_reg_) read/written like hardware |
| **Tick-based main loop** | `main.cpp` – Fixed 50 ms period, no blocking, mimics embedded superloop |
| **Deterministic timing** | `ms_t` clock, phase durations in ms, predictable behavior |
| **Bit packing** | RawInputs/Outputs could be packed into uint8/16; struct used for clarity |
| **Volatile semantics** | GPIO registers conceptually volatile (hardware can change them) |

---

## CS 128: C++ Programming

| Concept | Where in SmartSignal |
|---------|----------------------|
| **RAII** | `Logger` – Opens in open(), closes in destructor; no leak on early return |
| **Encapsulation** | Classes: TrafficFSM, GPIO, Debouncer, Logger – state hidden, API clear |
| **Const correctness** | get_outputs() const, read_inputs() const |
| **Namespaces** | `smartsignal::` for all public types |
| **enum class** | LampState, Phase, Event – type-safe, no implicit int conversion |
| **STL / standard library** | std::string, std::filesystem, std::fopen |
