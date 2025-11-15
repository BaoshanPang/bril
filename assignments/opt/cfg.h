#ifndef CFG_H_
#define CFG_H_
#include "block.h"
#include <map>

class cfg {
private:
  string  name;
  block_list blst;
  map<string, block> name2block;

public:
  cfg(json func) {
    name = func["name"];
    block *b = new block();
    for (auto instj : func["instrs"]) {
      instruction *inst = new instruction(instj);
      if (inst->is_label()) {
        if (b->get_count() > 0)
          this->append(b);
        b = new block();
        b->append(inst);
      } else {
        b->append(inst);
        if (inst->is_terminator()) {
          this->append(b);
          b = new block();
        }
      }
    }
    if (b->get_count() > 0)
      this->append(b);
  }
  void append(block *b) { blst.append(b); }
  json to_json() {
    json func;
    func["name"] = name;
    func["instrs"] = blst.to_json();
    return func;
  }
  void get_uses(set<string> &uses) {
    blst.get_uses(uses);
  }
  void dce();
  void dce_reassign() { blst.dce_reassign(); }
  void dump() { blst.dump(); }
};

#endif // CFG_H_
