#include "smartsignal/config.hpp"
#include "smartsignal/debounce.hpp"
#include "smartsignal/detection.hpp"
#include "smartsignal/events.hpp"
#include "smartsignal/fsm.hpp"
#include "smartsignal/gpio.hpp"
#include "smartsignal/logger.hpp"

#include <cstdio>
#include <filesystem>

// Simulates an approaching emergency vehicle: siren gets louder and higher
// in pitch from 8 to 13 seconds. Outside that window, ambient noise only.
static void generate_audio(smartsignal::AudioSample* buf, size_t n,
                          smartsignal::ms_t now_ms) {
  for (size_t i = 0; i < n; ++i) {
    if (now_ms >= 8000 && now_ms <= 13000) {
      float t = static_cast<float>(now_ms - 8000);
      buf[i].frequency_hz = 900.0f + 0.04f * t;
      buf[i].amplitude = 0.3f + 0.0001f * t;
    } else {
      buf[i].frequency_hz = 300.0f;
      buf[i].amplitude = 0.05f;
    }
  }
}

int main() {
  smartsignal::Config cfg;
  smartsignal::config_set_defaults(cfg);
  if (!smartsignal::config_load(cfg, "config/default.yaml")) {
    smartsignal::config_load(cfg, "../config/default.yaml");
  }

  smartsignal::TimingConfig timing;
  timing.green_ms = cfg.green_ms;
  timing.yellow_ms = cfg.yellow_ms;
  timing.all_red_ms = cfg.all_red_ms;

  smartsignal::TrafficFSM fsm;
  fsm.init(0);

  smartsignal::GPIO gpio;
  smartsignal::DebouncedInputs debounced;
  debounced.ped_ns.set_threshold(4);
  debounced.ped_ew.set_threshold(4);
  debounced.emergency.set_threshold(4);

  smartsignal::DetectionConfig det_cfg;
  det_cfg.siren_conf_threshold = cfg.siren_conf_threshold;
  det_cfg.trigger_samples = 6;
  det_cfg.clear_samples = 10;
  smartsignal::DetectionState det_state{};

  smartsignal::AudioSample audio_buf[16];

  smartsignal::Logger logger;
  std::filesystem::path log_path(cfg.log_filename);
  if (log_path.has_parent_path()) {
    std::filesystem::create_directories(log_path.parent_path());
  }
  if (!logger.open(cfg.log_filename)) {
    std::fprintf(stderr, "Warning: could not open %s for logging\n",
                 cfg.log_filename.c_str());
  }

  std::printf("SmartSignal - Traffic Light Controller Simulation\n");
  std::printf("  Duration: %u ms | Tick: %u ms | Log: %s\n\n",
              cfg.sim_duration_ms, cfg.tick_ms, cfg.log_filename.c_str());
  std::printf("  time_ms    NS      EW      top_event  siren  conf\n");
  std::printf("  ------    ---     ---     ---------  -----  ----\n");

  smartsignal::ms_t now_ms = 0;
  smartsignal::EventFlags flags;

  while (now_ms <= cfg.sim_duration_ms) {
    smartsignal::RawInputs raw;
    raw.ped_ns = false;
    if (now_ms >= 3000 && now_ms < 3500) {
      raw.ped_ns = true;
    }
    raw.ped_ew = false;
    raw.emergency = false;
    gpio.write_inputs(raw);

    debounced.tick(gpio.read_inputs());
    smartsignal::RawInputs clean = debounced.get();

    generate_audio(audio_buf, 16, now_ms);
    smartsignal::DetectionResult siren =
        smartsignal::detect_siren(audio_buf, 16, det_cfg, &det_state);

    flags = {};
    flags.active[static_cast<size_t>(smartsignal::Event::Timer)] = true;
    if (siren.detected) {
      flags.active[static_cast<size_t>(smartsignal::Event::Siren)] = true;
    }
    if (clean.ped_ns || clean.ped_ew) {
      flags.active[static_cast<size_t>(smartsignal::Event::PedButton)] = true;
    }
    if (clean.emergency) {
      flags.active[static_cast<size_t>(smartsignal::Event::Siren)] = true;
    }

    smartsignal::Event top = smartsignal::pick_highest_priority(flags);
    fsm.step(now_ms, flags, timing);

    smartsignal::Outputs out = fsm.get_outputs();
    gpio.write_outputs(out);

    logger.log_step(now_ms, out, top, siren.confidence, fsm.phase());

    const char* top_str = "OTHER";
    if (top == smartsignal::Event::None) {
      top_str = "NONE";
    } else if (top == smartsignal::Event::Siren) {
      top_str = "SIREN";
    } else if (top == smartsignal::Event::PedButton) {
      top_str = "PED";
    } else if (top == smartsignal::Event::Timer) {
      top_str = "TIMER";
    }

    const char* ns_str = "RED";
    if (out.ns == smartsignal::LampState::GREEN) {
      ns_str = "GREEN";
    } else if (out.ns == smartsignal::LampState::YELLOW) {
      ns_str = "YELLOW";
    }

    const char* ew_str = "RED";
    if (out.ew == smartsignal::LampState::GREEN) {
      ew_str = "GREEN";
    } else if (out.ew == smartsignal::LampState::YELLOW) {
      ew_str = "YELLOW";
    }

    int siren_flag = 0;
    if (siren.detected) {
      siren_flag = 1;
    }

    std::printf("  %6u    %-6s  %-6s  %-9s  %d      %.2f\n", now_ms, ns_str,
                ew_str, top_str, siren_flag, siren.confidence);

    now_ms += cfg.tick_ms;
  }

  logger.close();
  std::printf("\n  Done. Log written to %s\n", cfg.log_filename.c_str());
  return 0;
}
