#include "smartsignal/config.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace smartsignal {

// Trims leading and trailing whitespace from s.
static std::string trim(const std::string& s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  size_t end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

// Returns true if line is empty or starts with #.
static bool is_comment_or_empty(const std::string& line) {
  std::string t = trim(line);
  if (t.empty()) {
    return true;
  }
  if (t[0] == '#') {
    return true;
  }
  return false;
}

// Parses "key: value" or "key = value". Returns true if format is valid.
static bool parse_line(const std::string& line, std::string& key,
                       std::string& value) {
  size_t sep = line.find(':');
  if (sep == std::string::npos) {
    sep = line.find('=');
  }
  if (sep == std::string::npos) {
    return false;
  }

  key = trim(line.substr(0, sep));
  value = trim(line.substr(sep + 1));
  return !key.empty();
}

void config_set_defaults(Config& cfg) {
  cfg.green_ms = 5000;
  cfg.yellow_ms = 2000;
  cfg.all_red_ms = 1000;
  cfg.blue_min_hold_ms = 3000;
  cfg.tick_ms = 50;
  cfg.sim_duration_ms = 30000;
  cfg.siren_conf_threshold = 0.75f;
  cfg.log_filename = "logs/run.csv";
}

bool config_load(Config& cfg, const std::string& path) {
  std::ifstream f(path);
  if (!f) {
    return false;
  }

  std::string line;
  while (std::getline(f, line)) {
    if (is_comment_or_empty(line)) {
      continue;
    }

    std::string key, value;
    if (!parse_line(line, key, value)) {
      continue;
    }

    try {
      if (key == "green_ms") {
        cfg.green_ms = static_cast<unsigned int>(std::stoul(value));
      } else if (key == "yellow_ms") {
        cfg.yellow_ms = static_cast<unsigned int>(std::stoul(value));
      } else if (key == "all_red_ms") {
        cfg.all_red_ms = static_cast<unsigned int>(std::stoul(value));
      } else if (key == "blue_min_hold_ms") {
        cfg.blue_min_hold_ms = static_cast<unsigned int>(std::stoul(value));
      } else if (key == "tick_ms") {
        cfg.tick_ms = static_cast<unsigned int>(std::stoul(value));
      } else if (key == "sim_duration_ms") {
        cfg.sim_duration_ms = static_cast<unsigned int>(std::stoul(value));
      } else if (key == "siren_conf_threshold") {
        cfg.siren_conf_threshold = std::stof(value);
      } else if (key == "log_filename") {
        cfg.log_filename = value;
      }
    } catch (const std::exception&) {
      // Skip invalid values
    }
  }
  return true;
}

}  // namespace smartsignal
