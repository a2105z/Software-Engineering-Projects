#pragma once

#include "smartsignal/types.hpp"

#include <cstddef>

namespace smartsignal {

// Tunable parameters for siren detection.
struct DetectionConfig {
  // Confidence must exceed this to count as "siren present" for hysteresis.
  float siren_conf_threshold{0.75f};

  // Consecutive high-confidence readings needed to latch detected=true.
  int trigger_samples{6};

  // Consecutive low-confidence readings needed to clear detected=false.
  int clear_samples{10};
};

// Internal state for hysteresis. Persists across calls.
struct DetectionState {
  int consecutive_high{0};
  int consecutive_low{0};
  bool detected{false};
};

// Runs siren detection on a batch of audio samples. Uses hysteresis so we don't
// flip on/off from single noisy readings. Call every tick with fresh samples.
DetectionResult detect_siren(const AudioSample* samples, size_t count,
                             const DetectionConfig& config,
                             DetectionState* state);

}  // namespace smartsignal
