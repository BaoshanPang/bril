#include "cfg.h"
#include <set>

void cfg::dce() {
  set<string> uses;
  blst.get_uses(uses);
  blst.remove_dead_instructions(uses);
}
