#include "smartsignal/logger.hpp"

#include <cstdio>

namespace smartsignal {

static const char* lamp_str(LampState L) {
  switch (L) {
    case LampState::OFF:
      return "OFF";
    case LampState::RED:
      return "RED";
    case LampState::YELLOW:
      return "YELLOW";
    case LampState::GREEN:
      return "GREEN";
    case LampState::BLUE:
      return "BLUE";
  }
  return "?";
}

static const char* event_str(Event e) {
  switch (e) {
    case Event::None:
      return "NONE";
    case Event::Crash:
      return "CRASH";
    case Event::Siren:
      return "SIREN";
    case Event::ManualOverride:
      return "OVERRIDE";
    case Event::PedButton:
      return "PED";
    case Event::Timer:
      return "TIMER";
    case Event::Count:
      return "?";
  }
  return "?";
}

static const char* phase_str(Phase p) {
  switch (p) {
    case Phase::NS_GREEN:
      return "NS_GREEN";
    case Phase::NS_YELLOW:
      return "NS_YELLOW";
    case Phase::ALL_RED_1:
      return "ALL_RED_1";
    case Phase::EW_GREEN:
      return "EW_GREEN";
    case Phase::EW_YELLOW:
      return "EW_YELLOW";
    case Phase::ALL_RED_2:
      return "ALL_RED_2";
  }
  return "?";
}

bool Logger::open(const std::string& path) {
  close();
  fp_ = std::fopen(path.c_str(), "w");
  if (!fp_) {
    return false;
  }
  std::fprintf(fp_, "time_ms,ns,ew,ped,blue_beacon,top_event,siren_conf,phase\n");
  return true;
}

void Logger::log_step(ms_t time_ms, const Outputs& out, Event top_event,
                      float siren_conf, Phase phase) {
  if (!fp_) {
    return;
  }
  int blue_flag = 0;
  if (out.blue_beacon) {
    blue_flag = 1;
  }
  std::fprintf(fp_, "%u,%s,%s,%s,%d,%s,%.2f,%s\n", time_ms, lamp_str(out.ns),
               lamp_str(out.ew), lamp_str(out.ped), blue_flag,
               event_str(top_event), siren_conf, phase_str(phase));
}

void Logger::close() {
  if (fp_) {
    std::fclose(fp_);
    fp_ = nullptr;
  }
}

Logger::~Logger() {
  close();
}

}  // namespace smartsignal
