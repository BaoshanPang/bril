#ifndef CFG_H_
#define CFG_H_
#include "block.h"
#include <map>
#include <fstream>

class cfg {
private:
  string  name;
  block_list blst;
  name2block_map name2block;

public:
  cfg(json func) {
    name = func["name"];
    int cnt = 0;
    block *b = new block(cnt++);
    for (auto instj : func["instrs"]) {
      instruction *inst = new instruction(instj);
      if (inst->is_label()) {
        if (b->get_count() > 0)
          this->append(b);
        b = new block(cnt++, inst->get_label());
        b->append(inst);
      } else {
        b->append(inst);
        if (inst->is_terminator()) {
          this->append(b);
          b = new block(cnt++);
        }
      }
    }
    if (b->get_count() > 0)
      this->append(b);
    blst.gen_name_2_block_map(name2block);
    blst.set_relation(name2block);
    to_dot("cfg.dot");
  }
  void append(block *b) { blst.append(b); }
  json to_json() {
    json func;
    func["name"] = name;
    func["instrs"] = blst.to_json();
    return func;
  }
  void to_dot(string n) {
    ofstream fout(n);
    fout << "digraph {" << endl;
    blst.output_block_name(fout);
    blst.output_block_succs(fout, " -> ");
    fout << "}" << endl;
  }
  void get_uses(set<string> &uses) {
    blst.get_uses(uses);
  }
  void dce();
  void dce_reassign() { blst.dce_reassign(); }
  void lvn() { blst.lvn(); }

  void collect_dominators() { blst.collect_dominators(); }

  void reaching_defs();
  void dump() { blst.dump(); }
};

#endif // CFG_H_
