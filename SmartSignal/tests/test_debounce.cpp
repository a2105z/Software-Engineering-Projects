#include "smartsignal/debounce.hpp"

#include <cassert>
#include <cstdio>

int main() {
  smartsignal::Debouncer db;
  db.set_threshold(3);

  // Start low. Feed 3 high readings -> should go high.
  assert(db.get() == false);
  db.tick(true);
  assert(db.get() == false);
  db.tick(true);
  assert(db.get() == false);
  db.tick(true);
  assert(db.get() == true);

  // One low in the middle -> should stay high (we didn't get 3 lows yet).
  db.tick(false);
  assert(db.get() == true);
  db.tick(true);
  assert(db.get() == true);

  // Three lows -> should go low.
  db.tick(false);
  db.tick(false);
  db.tick(false);
  assert(db.get() == false);

  std::printf("test_debounce: OK\n");
  return 0;
}
