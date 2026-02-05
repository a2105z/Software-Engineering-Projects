#include "smartsignal/events.hpp"

namespace smartsignal {

// Priority order: first in list wins. Crash beats everything, Timer is lowest.
static constexpr Event PRIORITY_ORDER[] = {
    Event::Crash,
    Event::Siren,
    Event::ManualOverride,
    Event::PedButton,
    Event::Timer,
};

Event pick_highest_priority(const EventFlags& flags) {
  for (Event e : PRIORITY_ORDER) {
    size_t idx = static_cast<size_t>(e);
    if (flags.active[idx]) {
      return e;
    }
  }
  return Event::None;
}

}  // namespace smartsignal
