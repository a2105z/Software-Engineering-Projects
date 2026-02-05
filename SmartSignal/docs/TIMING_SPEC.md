# SmartSignal Timing Specification

## Tick-Based Execution

The main loop runs at a fixed **tick rate** (default 50 ms). Every tick:

1. Read/simulate inputs
2. Run detection
3. Debounce raw inputs
4. Arbitrate events
5. Step FSM
6. Write outputs
7. Log

This mimics a microcontroller's main loop: deterministic, periodic, no RTOS.

## Phase Timings (Default)

| Parameter      | Value   | Description                    |
|----------------|---------|--------------------------------|
| green_ms       | 5000    | NS/EW green phase duration     |
| yellow_ms      | 2000    | Yellow clearance duration      |
| all_red_ms     | 1000    | All-red clearance between flows|
| blue_min_hold_ms | 3000  | Min emergency hold (reserved)  |
| tick_ms        | 50      | Main loop period (20 Hz)       |
| sim_duration_ms | 30000 | Total simulation length        |

## Detection Timing

| Parameter           | Value | Description                          |
|---------------------|-------|--------------------------------------|
| siren_conf_threshold| 0.75  | Confidence to count as siren present |
| trigger_samples     | 6     | Consecutive highs to latch detected  |
| clear_samples       | 10    | Consecutive lows to clear            |

## Debounce Timing

Each debouncer uses a **sample count** threshold (default 4). At 50 ms/tick, 4 samples = 200 ms debounce time.
