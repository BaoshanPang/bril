#include "cfg.h"
#include "instruction.h"
#include <set>
#include <queue>

void cfg::dce() {
  bool changed = false;
  do {
    set<string> uses;
    blst.get_uses(uses);
    changed = blst.remove_dead_instructions(uses);
  } while(changed);
}

void cfg::reaching_defs() {
  blst.clear_out_defs();
  queue<block *> *worklist = blst.get_worklist();;
  while (!worklist->empty()) {
    block *b = worklist->front();
    worklist->pop();
    set<block *> *ps = b->get_preds();
    var2instset in_defs;
    for (auto &p : *ps) {
      for (auto &[var, si] : *p->get_out_defs()) {
        if (in_defs.find(var) == in_defs.end())
          in_defs[var] = si;
        else {
          in_defs[var].insert(si.begin(), si.end());
        }
      }
    }
    bool changed = b->update_out_defs(in_defs);
    if (changed) {
      set<block *> *ss = b->get_succs();
      for (auto &s : *ss) {
        worklist->push(s);
      }
    }
  }
}
