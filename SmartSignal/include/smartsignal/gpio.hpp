#pragma once

#include "smartsignal/types.hpp"

namespace smartsignal {

// Raw inputs from hardware pins. Before debouncing.
struct RawInputs {
  bool ped_ns{false};      // Pedestrian button North-South
  bool ped_ew{false};      // Pedestrian button East-West
  bool emergency{false};   // Emergency vehicle present
};

// Simulated GPIO register bank. Like memory-mapped I/O on a microcontroller.
// Input register is read by the CPU; output register is written to drive lamps.
class GPIO {
 public:
  // Reads the input register (what the pins are currently reporting).
  RawInputs read_inputs() const;

  // Writes raw values into the input register. Call from simulator to fake
  // button presses. In real hardware, this would be driven by actual pins.
  void write_inputs(const RawInputs& r);

  // Packs Outputs into the output register and returns what we "wrote".
  // In real hardware, this would drive the physical lamps.
  void write_outputs(const Outputs& out);

  // Reads back the last outputs we wrote. For debugging or verification.
  Outputs read_outputs() const;

 private:
  RawInputs input_reg_;
  Outputs output_reg_;
};

}  // namespace smartsignal
