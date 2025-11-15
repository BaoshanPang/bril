#include "cfg.h"
#include <set>

void cfg::dce() {
  bool changed = false;
  do {
    set<string> uses;
    blst.get_uses(uses);
    changed = blst.remove_dead_instructions(uses);
  } while(changed);
}
