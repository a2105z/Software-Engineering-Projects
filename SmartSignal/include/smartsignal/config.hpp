#pragma once

#include <string>

namespace smartsignal {

// All tunable settings for the simulator.
struct Config {
  // Phase timings (ms)
  unsigned int green_ms{5000};
  unsigned int yellow_ms{2000};
  unsigned int all_red_ms{1000};
  unsigned int blue_min_hold_ms{3000};

  // Simulation
  unsigned int tick_ms{50};
  unsigned int sim_duration_ms{30000};

  // Detection
  float siren_conf_threshold{0.75f};

  // Logging
  std::string log_filename{"logs/run.csv"};
};

// Fills cfg with sensible defaults.
void config_set_defaults(Config& cfg);

// Loads config from file. Returns true on success.
// Format: "key: value" per line. Unknown keys are ignored.
bool config_load(Config& cfg, const std::string& path);

}  // namespace smartsignal
