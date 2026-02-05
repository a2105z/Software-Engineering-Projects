#pragma once

#include <cstddef>

namespace smartsignal {

// Event IDs. Order here is not priority; arbiter decides that.
enum class Event : int {
  None = 0,
  Crash,           // Collision in intersection
  Siren,           // Emergency siren detected
  ManualOverride,  // Operator preemption
  PedButton,       // Pedestrian button pressed
  Timer,           // Time progression / phase timeout
  Count,
};

// Flags for which events are active this tick.
struct EventFlags {
  bool active[static_cast<size_t>(Event::Count)]{};
};

// Returns the highest-priority active event, or Event::None if none.
Event pick_highest_priority(const EventFlags& flags);

}  // namespace smartsignal
