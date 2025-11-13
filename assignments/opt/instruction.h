#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

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
  void dump() { cout << data << endl; }
  instruction *get_next() { return next; }
  void set_next(instruction *inst) { next = inst; }
  bool is_label() { return data.contains("label"); }
  bool is_terminator() {
    return data.contains("op") &&
      (data["op"] == "br" || data["op"] == "jmp");
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
      tail = inst;
    }
    count++;
  }
  size_t get_count() { return count;}
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
