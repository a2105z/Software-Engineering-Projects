#pragma once

#include "smartsignal/events.hpp"
#include "smartsignal/fsm.hpp"
#include "smartsignal/types.hpp"

#include <string>

namespace smartsignal {

// RAII logger. Opens file on construction, writes CSV rows, closes on destruction.
class Logger {
 public:
  Logger() = default;

  // Opens the log file. Returns false if open fails.
  bool open(const std::string& path);

  // Writes one CSV row for a simulation tick.
  void log_step(ms_t time_ms, const Outputs& out, Event top_event,
                float siren_conf, Phase phase);

  // Closes the file if open. Safe to call multiple times.
  void close();

  // Destructor closes the file.
  ~Logger();

  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  bool is_open() const { return fp_ != nullptr; }

 private:
  FILE* fp_{nullptr};
};

}  // namespace smartsignal
