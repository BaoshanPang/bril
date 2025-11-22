#ifndef BLOCK_H_
#define BLOCK_H_
#include <fstream>
#include "instruction.h"

class block;
typedef map<string, block *> name2block_map;

class block {
private:
  int idx;
  string name;
  inst_list ilst;
  block *prev;
  block *next;
  vector<block*> preds;
  vector<block *> succs;

public:
  block() { prev = next = nullptr; }
  block(int n) { prev = next = nullptr; idx = n; name = string("block_") + to_string(idx);}
  block(int n, string nm) { prev = next = nullptr; idx = n; name = nm;}
  string get_name() { return name; }
  int get_index() { return idx; }
  void append(instruction *inst) { ilst.append(inst);}
  void set_next(block *b) { next = b; }
  block* get_next() { return next; }
  size_t get_count() { return ilst.get_count(); }
  void get_uses(set<string> &uses) { ilst.get_uses(uses); }
  vector<block *>* get_succs() { return &succs; }
  void set_succs(name2block_map &n2b, block *n) {
    instruction *last = ilst.get_last_inst();
    if (last->is_jmp()) {
      string dest = last->get_labels()[0];
      succs.push_back(n2b[dest]);
    } else {
      if(n)
        succs.push_back(n);
      if (last->is_branch()) {
        json dests = last->get_labels();
        for (auto d : dests) {
          block *b = n2b[d];
          if(b != n)
            succs.push_back(b);
        }
      }
    }
  }
  bool remove_dead_instructions(const set<string> &uses) {
    return ilst.remove_dead_instructions(uses);
  }
  void remove_dead_assign() { ilst.remove_dead_assign(); }

  void lvn() { ilst.lvn(); }

  void dump_lvn() {
    cout << "block: " << endl;
    ilst.dump_lvn();
  }

  json to_json() {
    return ilst.to_json();
  }
  void dump() {
    cout << idx << " : " << name << endl;
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

  void get_uses(set<string> &uses) {
    block *b = head;
    while (b != nullptr) {
      b->get_uses(uses);
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
      vector<block *> *ss = b->get_succs();
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
      b = b->get_next();
    }
  }
};

#endif // BLOCK_H_
