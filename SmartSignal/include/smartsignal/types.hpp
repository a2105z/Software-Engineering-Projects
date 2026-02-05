#pragma once

#include <cstdint>

namespace smartsignal {

// Time in milliseconds since sim start. Typedef makes units explicit.
using ms_t = std::uint32_t;

// Lamp colors for vehicle and pedestrian signals.
enum class LampState : int {
  OFF = 0,
  RED,
  YELLOW,
  GREEN,
  BLUE,
};

// Output state of the intersection. What the hardware would drive each tick.
struct Outputs {
  LampState ns;      // North-South vehicle light
  LampState ew;      // East-West vehicle light
  LampState ped;     // Pedestrian light (GREEN = walk)
  bool blue_beacon;  // Emergency blue indicator
};

// Result from a detector (e.g. siren). Separates decision from confidence.
struct DetectionResult {
  bool detected;    // Latched detection decision
  float confidence; // 0.0 to 1.0
};

// Single processed audio sample for detection input.
struct AudioSample {
  float frequency_hz; // Dominant frequency
  float amplitude;    // Normalized 0.0 to 1.0
};

}  // namespace smartsignal
