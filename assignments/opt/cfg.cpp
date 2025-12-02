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

  blst.gen_var2blks_map(var2blks);
}

void cfg::ssa_insert_phi() {
  bool changed = false;
  do {
    for (auto &[v, bs] : var2blks) {
      for (auto &b : bs) {
        for (auto &df1 : *b->get_df()) {
          df1->insert_phi(v);
          if (var2blks.find(v) == var2blks.end()) {
            var2blks[v].insert(df1);
            changed = true;
          }
        }
      }
    }
  } while (changed);
}

void cfg::ssa_init_name_stack() {
  for (auto &[v, bs] : var2blks) {
    stack<string> stk;
    stk.push(v);
    namestack[v] = make_tuple(0, stk);
  }
}


void cfg::ssa_rename() {
  ssa_init_name_stack();
  blst.ssa_rename(namestack);
}

void cfg::ssa_remove_phi() {
   blst.ssa_remove_phi();
}
