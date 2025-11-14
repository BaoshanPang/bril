#ifndef BLOCK_H_
#define BLOCK_H_
#include "instruction.h"

class block {
private:
  inst_list ilst;
  block *prev;
  block *next;
  vector<block*> preds;
  vector<block*> succs;
public:
  block() { prev = next = nullptr; }
  void append(instruction *inst) { ilst.append(inst);}
  void set_next(block *b) { next = b; }
  block* get_next() { return next; }
  size_t get_count() { return ilst.get_count(); }
  void get_uses(set<string> &uses) { ilst.get_uses(uses); }
  void remove_dead_instructions(const set<string> &uses) {
    ilst.remove_dead_instructions(uses);
  }
  json to_json() {
    return ilst.to_json();
  }
  void dump() {
    cout << "block: " << endl;
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

  void remove_dead_instructions(const set<string> &uses) {
     block *b = head;
     while (b != nullptr) {
      b->remove_dead_instructions(uses);
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
