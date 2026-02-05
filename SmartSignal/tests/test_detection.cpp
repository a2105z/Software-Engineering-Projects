#include "smartsignal/detection.hpp"

#include <cassert>
#include <cstdio>

// Build a batch of samples that look like a siren (freq in range, good amplitude).
static void make_siren_samples(smartsignal::AudioSample* buf, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    buf[i].frequency_hz = 900.0f;
    buf[i].amplitude = 0.5f;
  }
}

// Build a batch that looks like ambient noise (low amplitude, wrong freq).
static void make_noise_samples(smartsignal::AudioSample* buf, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    buf[i].frequency_hz = 300.0f;
    buf[i].amplitude = 0.05f;
  }
}

int main() {
  smartsignal::DetectionConfig cfg;
  cfg.siren_conf_threshold = 0.75f;
  cfg.trigger_samples = 4;
  cfg.clear_samples = 4;

  smartsignal::DetectionState state{};
  smartsignal::AudioSample buf[8];

  // Feed siren-like samples. After 4 ticks, detected should go true.
  for (int i = 0; i < 4; ++i) {
    make_siren_samples(buf, 8);
    smartsignal::DetectionResult r =
        smartsignal::detect_siren(buf, 8, cfg, &state);
    if (i < 3) {
      assert(!r.detected);
    } else {
      assert(r.detected);
    }
  }

  // Feed noise. After 4 ticks, detected should go false.
  for (int i = 0; i < 4; ++i) {
    make_noise_samples(buf, 8);
    smartsignal::DetectionResult r =
        smartsignal::detect_siren(buf, 8, cfg, &state);
    if (i < 3) {
      assert(r.detected);
    } else {
      assert(!r.detected);
    }
  }

  std::printf("test_detection: OK\n");
  return 0;
}
