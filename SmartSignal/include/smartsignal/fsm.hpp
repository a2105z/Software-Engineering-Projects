#pragma once

#include "smartsignal/events.hpp"
#include "smartsignal/types.hpp"

namespace smartsignal {

// Phase timings in milliseconds. Passed in so FSM stays configurable.
struct TimingConfig {
  unsigned int green_ms{5000};
  unsigned int yellow_ms{2000};
  unsigned int all_red_ms{1000};
};

// Traffic phases. Normal cycle: NS_GREEN -> NS_YELLOW -> ALL_RED_1 -> EW_GREEN ->
// EW_YELLOW -> ALL_RED_2 -> back to NS_GREEN.
enum class Phase {
  NS_GREEN,      // North-South has green
  NS_YELLOW,     // North-South clearing
  ALL_RED_1,     // Brief all-red between NS and EW
  EW_GREEN,      // East-West has green
  EW_YELLOW,     // East-West clearing
  ALL_RED_2,     // Brief all-red between EW and NS
};

// Finite state machine for the traffic lights.
class TrafficFSM {
 public:
  // Sets phase to NS_GREEN and resets timing.
  void init(ms_t now_ms);

  // Advances the FSM. Call every tick with current time and events.
  void step(ms_t now_ms, const EventFlags& events, const TimingConfig& timing);

  // Returns lamp outputs for the current phase.
  Outputs get_outputs() const;

  Phase phase() const;

 private:
  Phase phase_{Phase::NS_GREEN};
  ms_t entered_at_ms_{0};

  // How long we've been in the current phase.
  ms_t time_in_phase(ms_t now_ms) const;
};

}  // namespace smartsignal
