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
  var2blks_map var2blks;

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

  void create_dom_tree() { blst.create_dom_tree(); }

  void get_dom_frontier() { blst.get_df(); }

  void reaching_defs();

  // SSA
  void ssa_insert_phi();
  void dump() {
    blst.dump();

    cout << "\n// var2blks:" << endl;
    for (auto &[v, bs] : var2blks) {
      cout << "// " << v << " : ";
      for (auto b : bs) {
        cout << b->get_name() << ",";
      }
      cout << endl;
    }
  }

  void dom_tree_to_dot() { blst.dom_tree_to_dot(); }
};

#endif // CFG_H_
