#include "cfg.h"

auto main(int argc, char *argv[]) -> int {
  json j;
  std::cin >> j;
  json funcs = json::array();
  for (auto func : j["functions"]) {
    cfg *g = new cfg(func);
#ifndef LVN
    g->dce();
    g->dce_reassign();
#else
    g->reaching_defs();
    g->lvn();
    g->dce();
    g->dce_reassign();
    g->collect_dominators();
    g->get_dom_frontier();
    g->create_dom_tree();
    g->ssa_insert_phi();
    g->ssa_rename();
    g->dump();
    g->dom_tree_to_dot();
#endif
    funcs.push_back(g->to_json());
  }
  json out;
  out["functions"] = funcs;
  cout << out.dump(2) << endl;
  return 0;
}
