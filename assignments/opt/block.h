#ifndef BLOCK_H_
#define BLOCK_H_
#include "instruction.h"
#include <cmath>
#include <fstream>
#include <queue>

class block;
typedef set<block *> block_set;
typedef map<string, block *> name2block_map;
typedef map<string, set<block *>> var2blks_map;

class block {
private:
  int idx;
  string name;
  inst_list ilst;
  block *prev;
  block *next;
  set<block *> preds;
  set<block *> succs;
  var2instset out_defs;
  var2instset defs; //
  set<block *> doms;
  block *idom = nullptr;
  set<block *> children_of_dom;
  block_set df; // dominator fronter
  // SSA
  map<string, instruction *> var2phi;
public:
  block() { prev = next = nullptr; }
  block(int n) {
    prev = next = nullptr;
    idx = n;
    name = string("block_") + to_string(idx);
  }
  block(int n, string nm) {
    prev = next = nullptr;
    idx = n;
    name = nm;
  }

  void insert_phi(const string &v) {
    if (var2phi.find(v) != var2phi.end())
      return;
    json phi;
    phi["op"] = "phi";
    phi["type"] = "int";
    phi["dest"] = v;
    instruction *i = new instruction(phi);
    ilst.insert(i);
    var2phi[v] = i;
  }

  void set_dom_self() { doms.insert(this); }

  string get_name() { return name; }
  int get_index() { return idx; }
  void append(instruction *inst) { ilst.append(inst); }
  void set_next(block *b) { next = b; }
  block *get_next() { return next; }
  size_t get_count() { return ilst.get_count(); }
  void get_uses(set<string> &uses) { ilst.get_uses(uses); }
  void set_pred(block *p) { preds.insert(p); }
  set<block *> *get_preds() { return &preds; }
  set<block *> *get_succs() { return &succs; }
  void set_succs(name2block_map &n2b, block *n) {
    instruction *last = ilst.get_last_inst();
    if (last->is_jmp()) {
      string dest = last->get_labels()[0];
      succs.insert(n2b[dest]);
    } else {
      if (n)
        succs.insert(n);
      if (last->is_branch()) {
        json dests = last->get_labels();
        for (auto d : dests) {
          block *b = n2b[d];
          if (b != n)
            succs.insert(b);
        }
      }
    }
  }
  bool remove_dead_instructions(const set<string> &uses) {
    return ilst.remove_dead_instructions(uses);
  }
  void remove_dead_assign() { ilst.remove_dead_assign(); }

  void lvn() { ilst.lvn(); }

  void clear_out_defs() { out_defs.clear(); }

  void gen_defs() {
    defs.clear();
    ilst.get_defs(defs);
  }

  bool update_out_defs(var2instset &in_defs) {
    gen_defs();
    for (auto &[var, si] : defs) {
      in_defs[var] = si;
    }
    bool changed = (out_defs.size() != in_defs.size()) ||
                   !equal(out_defs.begin(), out_defs.end(), in_defs.begin());
    if (changed) {
      out_defs = in_defs;
    }
    return changed;
  }

  var2instset *get_out_defs() { return &out_defs; }


  void gen_var2blks_map(var2blks_map &v2bs) {
    for (auto &[v, bs] : defs) {
      if (v2bs.find(v) == v2bs.end()) {
        block_set bs;
        bs.insert(this);
        v2bs[v] = bs;
      } else {
        v2bs[v].insert(this);
      }
    }
  }

  set<block *> *get_dom() { return &doms; }

  block *get_idom() {
    if (idom != nullptr)
      return idom;
    else {
      block *id = nullptr;
      for (auto &b : doms) {
        if (b == this)
          continue;
        if (id == nullptr)
          id = b;
        else if (b->get_dom()->size() > id->get_dom()->size()) {
          id = b;
        }
      }
      return id;
    }
  }

  bool collect_dominators() {
    int orig_size = doms.size();
    bool inited = false;
    for (auto &p : preds) {
      set<block *> *pd = p->get_dom();
      if(pd->size() == 0) continue;
      if (!inited) {
        doms = *pd;
        inited = true;
      } else {
        for (auto it = doms.begin(); it != doms.end();) {
          if (pd->count(*it) == 0) {
            it = doms.erase(it);
          } else
            ++it;
        }
      }
    }
    doms.insert(this);
    return doms.size() != orig_size;
  }

  void add_dom_child(block *b) { children_of_dom.insert(b); }

  set<block *>* get_dom_children() { return &children_of_dom; }

  void find_df() {
    queue<block *> wklist;
    set<block *> processed;

    wklist.push(this);
    processed.insert(this);
    while (!wklist.empty()) {
      block *b = wklist.front();
      wklist.pop();

      set<block *> *ss = b->get_succs();
      for (auto &s : *ss) {
        if(processed.count(s) != 0) continue;
        if (s->get_dom()->count(this) != 0) {
          wklist.push(s);
          processed.insert(s);
        } else {
          df.insert(s);
        }
      }
    }
  }

  block_set* get_df() { return &df; }

  void ssa_update_phi(block *pred, name_stack_map &nsm) {
    for (auto &[v, p] : var2phi) {
      auto s = pred->get_out_defs();
      if(s->find(v) != s->end())
        p->add_arg_and_label(get<1>(nsm[v]).top(), pred->get_name());
    }
  }

  void ssa_rename(name_stack_map &nsm) {
    name_stack_map nsm0 = nsm;
    ilst.ssa_rename(nsm0);
    for (auto &s : succs) {
      s->ssa_update_phi(this, nsm0);
    }
    for (auto b : children_of_dom) {
      b->ssa_rename(nsm0);
    }
  }

  void dump_lvn() {
    cout << "block: " << endl;
    ilst.dump_lvn();
  }

  json to_json() { return ilst.to_json(); }
  void dump() {
    cout << "// block_" <<  idx << " : " << name << endl;
    cout << "// dominators: ";
    for (auto &b : doms) {
      cout << b->get_name() << ",";
    }
    cout << endl;
    cout << "// dominator frontier: ";
    for (auto &b : df) {
      cout << b->get_name() << ",";
    }
    cout << endl;

    cout << "// idom: " << (idom == nullptr ? "null" : idom->get_name())
         << endl;

    cout << "// defs: ";
    for (auto &[v, is] : defs) {
      cout << v << ",";
    }
    cout << endl;

    cout << "// out_defs: ";
    for (auto &[v, is] : out_defs) {
      cout << v << ",";
    }
    cout << endl;

    cout << "// inst: " << endl;
    ilst.dump();
  }
};

class block_list {
private:
  block *head;
  block *tail;

public:
  block_list() { head = tail = nullptr; }
  void append(block *b) {
    if (head == nullptr) {
      head = tail = b;
    } else {
      tail->set_next(b);
      tail = b;
    }
  }

  void clear_out_defs() {
    block *b = head;
    while (b != nullptr) {
      b->clear_out_defs();
      b = b->get_next();
    }
  }

  queue<block *> *get_worklist() {
    queue<block *> *worklist = new queue<block *>();
    block *b = head;
    while (b != nullptr) {
      worklist->push(b);
      b = b->get_next();
    }
    return worklist;
  }

  void get_uses(set<string> &uses) {
    block *b = head;
    while (b != nullptr) {
      b->get_uses(uses);
      b = b->get_next();
    }
  }

  void gen_var2blks_map(var2blks_map &v2bs) {
    block *b = head;
    while (b != nullptr) {
      b->gen_var2blks_map(v2bs);
      b = b->get_next();
    }
  }

  void gen_name_2_block_map(name2block_map &n2b) {
    block *b = head;
    while (b != nullptr) {
      string name = b->get_name();
      assert(n2b.find(name) == n2b.end());
      n2b[b->get_name()] = b;
      b = b->get_next();
    }
  }

  void set_relation(name2block_map &n2b) {
    block *b = head;
    while (b != nullptr) {
      b->set_succs(n2b, b->get_next());
      for (auto &s : *b->get_succs()) {
        s->set_pred(b);
      }
      b = b->get_next();
    }
  }

  void output_block_name(ofstream &os) {
    block *b = head;
    while (b != nullptr) {
      os << b->get_name() << endl;
      b = b->get_next();
    }
  }

  void output_block_succs(ofstream &os, string deli) {
    block *b = head;
    while (b != nullptr) {
      string n = b->get_name();
      set<block *> *ss = b->get_succs();
      for (auto &s : *ss) {
        os << b->get_name() << deli << s->get_name() << endl;
      }
      b = b->get_next();
    }
  }

  bool remove_dead_instructions(const set<string> &uses) {
    bool changed = false;
    block *b = head;
    while (b != nullptr) {
      if (b->remove_dead_instructions(uses))
        changed = true;
      b = b->get_next();
    }
    return changed;
  }

  void dce_reassign() {
    block *b = head;
    while (b != nullptr) {
      b->remove_dead_assign();
      b = b->get_next();
    }
  }

  void lvn() {
    block *b = head;
    while (b != nullptr) {
      b->lvn();
      b = b->get_next();
    }
  }

  void get_idom() {
    block *b = head;
    while (b != nullptr) {
      b->get_idom();
      b = b->get_next();
    }
  }

  void collect_dominators() {
    bool changed = false;
    head->set_dom_self();
    do {
      changed = false;
      block *b = head->get_next();
      while (b != nullptr) {
        if (b->collect_dominators())
          changed = true;
        b = b->get_next();
      }
    } while (changed);
    get_idom();
  }

  void create_dom_tree() {
    block *b = head;
    while (b != nullptr) {
      block *ib = b->get_idom();
      if(ib)
        ib->add_dom_child(b);
      b = b->get_next();
    }
  }

  void get_df() {
    block *b = head;
    while (b != nullptr) {
      b->find_df();
      b = b->get_next();
    }
  }

  void ssa_rename(name_stack_map &nsm) {
    head->ssa_rename(nsm);
  }

  void dom_tree_to_dot() {
    ofstream fout("dom.dot");
    fout << "digraph {" << endl;
    output_block_name(fout);
    block *b = head;
    while (b != nullptr) {
      for (auto &b1 : *b->get_dom_children()) {
        fout << b->get_name() << " -> " << b1->get_name() << endl;
      }
      b = b->get_next();
    }
    fout << "}" << endl;
  }

  json to_json() {
    json insts = json::array();
    block *b = head;
    while (b != nullptr) {
      json insts0 = b->to_json();
      insts.insert(insts.end(), insts0.begin(), insts0.end());
      b = b->get_next();
    }
    return insts;
  }

  void dump() {
    block *b = head;
    while (b != nullptr) {
      b->dump();
      cout << endl;
      b = b->get_next();
    }
  }
};

#endif // BLOCK_H_
