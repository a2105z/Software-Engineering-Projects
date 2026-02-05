#include "smartsignal/fsm.hpp"

namespace smartsignal {

void TrafficFSM::init(ms_t now_ms) {
  phase_ = Phase::NS_GREEN;
  entered_at_ms_ = now_ms;
}

ms_t TrafficFSM::time_in_phase(ms_t now_ms) const {
  return now_ms - entered_at_ms_;
}

Phase TrafficFSM::phase() const {
  return phase_;
}

// How long each phase should last before we move on.
static unsigned int phase_duration(Phase p, const TimingConfig& t) {
  switch (p) {
    case Phase::NS_GREEN:
    case Phase::EW_GREEN:
      return t.green_ms;
    case Phase::NS_YELLOW:
    case Phase::EW_YELLOW:
      return t.yellow_ms;
    case Phase::ALL_RED_1:
    case Phase::ALL_RED_2:
      return t.all_red_ms;
  }
  return t.green_ms;
}

// Next phase in the normal cycle.
static Phase next_phase(Phase p) {
  switch (p) {
    case Phase::NS_GREEN:
      return Phase::NS_YELLOW;
    case Phase::NS_YELLOW:
      return Phase::ALL_RED_1;
    case Phase::ALL_RED_1:
      return Phase::EW_GREEN;
    case Phase::EW_GREEN:
      return Phase::EW_YELLOW;
    case Phase::EW_YELLOW:
      return Phase::ALL_RED_2;
    case Phase::ALL_RED_2:
      return Phase::NS_GREEN;
  }
  return Phase::NS_GREEN;
}

void TrafficFSM::step(ms_t now_ms, const EventFlags& events,
                      const TimingConfig& timing) {
  bool timer_active =
      events.active[static_cast<size_t>(Event::Timer)];

  if (!timer_active) {
    return;
  }

  ms_t elapsed = time_in_phase(now_ms);
  unsigned int limit = phase_duration(phase_, timing);

  if (elapsed >= limit) {
    phase_ = next_phase(phase_);
    entered_at_ms_ = now_ms;
  }
}

Outputs TrafficFSM::get_outputs() const {
  Outputs out{};
  out.ns = LampState::RED;
  out.ew = LampState::RED;
  out.ped = LampState::RED;
  out.blue_beacon = false;

  switch (phase_) {
    case Phase::NS_GREEN:
      out.ns = LampState::GREEN;
      out.ew = LampState::RED;
      break;
    case Phase::NS_YELLOW:
      out.ns = LampState::YELLOW;
      out.ew = LampState::RED;
      break;
    case Phase::ALL_RED_1:
    case Phase::ALL_RED_2:
      out.ns = LampState::RED;
      out.ew = LampState::RED;
      break;
    case Phase::EW_GREEN:
      out.ns = LampState::RED;
      out.ew = LampState::GREEN;
      break;
    case Phase::EW_YELLOW:
      out.ns = LampState::RED;
      out.ew = LampState::YELLOW;
      break;
  }

  return out;
}

}  // namespace smartsignal
