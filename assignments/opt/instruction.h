#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <set>

using namespace std;
using json = nlohmann::json;

class instruction {
private:
  json data;
  instruction *prev;
  instruction *next;

public:
  instruction(json inst) {
    data = inst;
    prev = next = nullptr;
  }
  json get_data() { return data; }
  instruction *get_prev() { return prev; }
  instruction *get_next() { return next; }
  void _next(instruction *inst) { next = inst; }
  void set_prev(instruction *inst) { prev = inst; }
  void set_next(instruction *inst) { next = inst; }
  bool is_label() { return data.contains("label"); }
  bool is_terminator() {
    return data.contains("op") &&
      (data["op"] == "br" || data["op"] == "jmp");
  }
  void get_uses(set<string> &uses) {
    if (data.contains("args")) {
      for (auto a : data["args"]) {
        uses.insert(a);
      }
    }
  }
  string get_def() {
    if (data.contains("dest"))
      return data["dest"];
    else
      return "";
  }
  bool is_dead(const set<string> &uses) {
    if (is_label())
      return false;
    if(data.contains("dest"))
      return uses.count(data["dest"]) == 0;
    return false;
  }
  void dump() {
    cout << data << endl;
  }
};

class inst_list {
private:
  instruction *head;
  instruction *tail;
  size_t count;

public:
  inst_list() { head = tail = nullptr; count = 0; }
  void append(instruction *inst) {
    if (head == nullptr) {
      head = tail = inst;
    } else {
      tail->set_next(inst);
      inst->set_prev(tail);
      tail = inst;
    }
    count++;
  }
  size_t get_count() { return count;}
  void get_uses(set<string> &uses) {
     instruction *i = head;
     while (i != nullptr) {
      i->get_uses(uses);
      i = i->get_next();
    }
  }

  bool remove_dead_instructions(const set<string> &uses) {
    bool changed = false;
    instruction *i = head;
    instruction *prev = head;
    while (i != nullptr) {
      if (i->is_dead(uses)) {
        if (i == head) {
          i = head = head->get_next();
        } else {
          i = i->get_next();
          prev->set_next(i);
        }
        changed = true;
      } else {
        prev = i;
        i = i->get_next();
      }
    }
    return changed;
  }

  void unlink(instruction *inst) {
    if (head == inst) {
      head = inst->get_next();
    } else {
      instruction *ip = inst->get_prev();
      instruction *in = inst->get_next();

      ip->set_next(in);
      in->set_prev(ip);
    }
  }

  void remove_dead_assign() {
    map<string, instruction *> d2i;
    set<string> uses;
    vector<instruction *> dead;
    instruction *i = head;
    while (i != nullptr) {
      i->get_uses(uses);
      string dest = i->get_def();
      if (dest != "") {
        if (uses.count(dest) == 0 && d2i.find(dest) != d2i.end()) {
          dead.push_back(d2i[dest]);
        }
        d2i[dest] = i;
        uses.erase(dest);
      }
      i = i->get_next();
    }
    for (auto &d : dead) {
      unlink(d);
    }
  }

  json to_json() {
    instruction *i = head;
    json insts = json::array();
    while (i != nullptr) {
      insts.push_back(i->get_data());
      i = i->get_next();
    }
    return insts;
  }
  void dump() {
    instruction *i = head;
    while (i != nullptr) {
      i->dump();
      i = i->get_next();
    }
  }
};

#endif // INSTRUCTION_H_
