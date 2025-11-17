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
    g->lvn();
    g->dce();
    g->dce_reassign();
#endif
    funcs.push_back(g->to_json());
  }
  json out;
  out["functions"] = funcs;
  cout << out.dump(2) << endl;
  return 0;
}
