#pragma once

#include "smartsignal/gpio.hpp"

namespace smartsignal {

// One debouncer for a single input. Needs N consecutive same readings before
// the output flips. Filters out button bounce and noise.
class Debouncer {
 public:
  // How many consecutive same readings needed to accept a change.
  void set_threshold(int n);

  // Feed a new raw reading. Call every tick.
  void tick(bool raw);

  // Current debounced value.
  bool get() const;

  // Reset to a known state. Use when starting the sim.
  void reset(bool value = false);

 private:
  int threshold_{4};
  int count_{0};
  bool last_raw_{false};
  bool output_{false};
};

// Holds three debouncers for the traffic inputs.
struct DebouncedInputs {
  Debouncer ped_ns;
  Debouncer ped_ew;
  Debouncer emergency;

  // Feed raw values and update all three debouncers.
  void tick(const RawInputs& raw);

  // Get debounced values.
  RawInputs get() const;
};

}  // namespace smartsignal
