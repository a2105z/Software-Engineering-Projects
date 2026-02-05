# SmartSignal State Diagram

## Traffic Phase FSM

The controller cycles through six phases. Each phase has a fixed duration (configurable via `config/default.yaml`).

```
                    +------------------+
                    |                  |
                    v                  |
    +----------+  ALL_RED_2  +----------+
    | NS_GREEN | ----------> | EW_GREEN |
    +----------+             +----------+
         |                        |
         | NS_YELLOW              | EW_YELLOW
         v                        v
    +----------+             +----------+
    |NS_YELLOW |             |EW_YELLOW |
    +----------+             +----------+
         |                        |
         | ALL_RED_1              |
         v                        |
    +----------+                  |
    |ALL_RED_1 | -----------------+
    +----------+
```

## Phase Descriptions

| Phase       | NS Lamp | EW Lamp | Duration  |
|------------|---------|---------|-----------|
| NS_GREEN   | GREEN   | RED     | green_ms  |
| NS_YELLOW  | YELLOW  | RED     | yellow_ms |
| ALL_RED_1  | RED     | RED     | all_red_ms|
| EW_GREEN   | RED     | GREEN   | green_ms  |
| EW_YELLOW  | RED     | YELLOW  | yellow_ms |
| ALL_RED_2  | RED     | RED     | all_red_ms|

## Transition Trigger

Transitions occur when:
1. **Timer** event is active, and
2. Time in current phase >= phase duration

Higher-priority events (Siren, PedButton, etc.) are passed to the FSM but do not change phase in the current implementation. Future versions may add emergency preemption and pedestrian phases.
