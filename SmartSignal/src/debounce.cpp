#include "smartsignal/debounce.hpp"

namespace smartsignal {

void Debouncer::set_threshold(int n) {
  if (n > 0) {
    threshold_ = n;
  } else {
    threshold_ = 1;
  }
}

bool Debouncer::get() const {
  return output_;
}

void Debouncer::tick(bool raw) {
  if (raw == output_) {
    count_ = 0;
    return;
  }
  if (raw == last_raw_) {
    count_++;
    if (count_ >= threshold_) {
      output_ = raw;
      count_ = 0;
    }
  } else {
    last_raw_ = raw;
    count_ = 1;
  }
}

void Debouncer::reset(bool value) {
  output_ = value;
  last_raw_ = value;
  count_ = 0;
}

void DebouncedInputs::tick(const RawInputs& raw) {
  ped_ns.tick(raw.ped_ns);
  ped_ew.tick(raw.ped_ew);
  emergency.tick(raw.emergency);
}

RawInputs DebouncedInputs::get() const {
  RawInputs r;
  r.ped_ns = ped_ns.get();
  r.ped_ew = ped_ew.get();
  r.emergency = emergency.get();
  return r;
}

}  // namespace smartsignal
