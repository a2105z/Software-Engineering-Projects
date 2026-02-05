#include "smartsignal/fsm.hpp"
#include "smartsignal/types.hpp"

#include <cassert>
#include <cstdio>

int main() {
  smartsignal::TrafficFSM fsm;
  smartsignal::TimingConfig timing;
  timing.green_ms = 100;   // Short for fast test
  timing.yellow_ms = 50;
  timing.all_red_ms = 50;

  smartsignal::EventFlags flags;
  flags.active[static_cast<size_t>(smartsignal::Event::Timer)] = true;

  fsm.init(0);

  // At start, NS should be green, EW red.
  smartsignal::Outputs out = fsm.get_outputs();
  assert(out.ns == smartsignal::LampState::GREEN);
  assert(out.ew == smartsignal::LampState::RED);

  // Step until we leave NS_GREEN (100ms). At t=100 we transition.
  for (smartsignal::ms_t t = 50; t <= 150; t += 50) {
    fsm.step(t, flags, timing);
  }
  out = fsm.get_outputs();
  assert(out.ns == smartsignal::LampState::YELLOW);
  assert(out.ew == smartsignal::LampState::RED);

  // Step until we hit EW_GREEN. NS_YELLOW=50, ALL_RED_1=50.
  for (smartsignal::ms_t t = 200; t <= 300; t += 50) {
    fsm.step(t, flags, timing);
  }
  out = fsm.get_outputs();
  assert(out.ns == smartsignal::LampState::RED);
  assert(out.ew == smartsignal::LampState::GREEN);

  std::printf("test_fsm: OK\n");
  return 0;
}
