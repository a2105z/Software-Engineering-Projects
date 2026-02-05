#include "smartsignal/detection.hpp"

#include <algorithm>
#include <cmath>

namespace smartsignal {

// Emergency sirens are typically 500-2000 Hz. Outside this range we treat as noise.
static constexpr float SIREN_FREQ_MIN_HZ = 500.0f;
static constexpr float SIREN_FREQ_MAX_HZ = 2000.0f;

// Amplitude below this is considered background. Sirens are loud.
static constexpr float SIREN_AMPLITUDE_MIN = 0.15f;

// Returns 0.0 to 1.0: how much this single sample looks like a siren.
static float sample_confidence(const AudioSample& s) {
  bool freq_ok = (s.frequency_hz >= SIREN_FREQ_MIN_HZ &&
                  s.frequency_hz <= SIREN_FREQ_MAX_HZ);
  bool amp_ok = (s.amplitude >= SIREN_AMPLITUDE_MIN);

  if (!freq_ok || !amp_ok) {
    return 0.0f;
  }

  // Blend: stronger amplitude and center-of-range freq score higher.
  float freq_score = 1.0f - std::abs(s.frequency_hz - 1000.0f) / 1000.0f;
  if (freq_score < 0.0f) {
    freq_score = 0.0f;
  }
  float amp_score =
      (s.amplitude - SIREN_AMPLITUDE_MIN) / (1.0f - SIREN_AMPLITUDE_MIN);
  if (amp_score > 1.0f) {
    amp_score = 1.0f;
  }

  return 0.5f * freq_score + 0.5f * amp_score;
}

// Average confidence over the batch. One batch = one tick's worth of samples.
static float batch_confidence(const AudioSample* samples, size_t count) {
  if (count == 0) {
    return 0.0f;
  }
  float sum = 0.0f;
  for (size_t i = 0; i < count; ++i) {
    sum += sample_confidence(samples[i]);
  }
  return sum / static_cast<float>(count);
}

DetectionResult detect_siren(const AudioSample* samples, size_t count,
                             const DetectionConfig& config,
                             DetectionState* state) {
  if (state == nullptr) {
    DetectionResult r;
    r.detected = false;
    r.confidence = 0.0f;
    return r;
  }

  float conf = batch_confidence(samples, count);

  if (conf >= config.siren_conf_threshold) {
    state->consecutive_high++;
    state->consecutive_low = 0;
    if (state->consecutive_high >= config.trigger_samples) {
      state->detected = true;
    }
  } else {
    state->consecutive_low++;
    state->consecutive_high = 0;
    if (state->consecutive_low >= config.clear_samples) {
      state->detected = false;
    }
  }

  DetectionResult r;
  r.detected = state->detected;
  r.confidence = conf;
  return r;
}

}  // namespace smartsignal
