#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_
#include <iostream>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
using json = nlohmann::json;

class instruction;
typedef map<string, set<instruction *>> var2instset;

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
  bool is_jmp() { return data.contains("op") && data["op"] == "jmp"; }
  bool is_branch() { return data.contains("op") && data["op"] == "br"; }
  string get_label() { return data["label"]; }
  json get_labels() { return data["labels"]; }
  bool is_terminator() {
    return data.contains("op") && (data["op"] == "br" || data["op"] == "jmp");
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
  void update_def(string dest) { data["dest"] = dest; }
  string get_op() {
    if (data.contains("op"))
      return data["op"];
    else
      return "";
  }
  bool has_args() { return data.contains("args"); }
  vector<string> get_args() {
    vector<string> args;
    if (data.contains("args")) {
      for (auto a : data["args"]) {
        args.push_back(a);
      }
    }
    return args;
  }
  bool is_dead(const set<string> &uses) {
    if (is_label())
      return false;
    if (data.contains("dest"))
      return uses.count(data["dest"]) == 0;
    return false;
  }
  void update_to_copy(string id) {
    if (data["op"] == "add" || data["op"] == "mul") {
      data["op"] = "id";
      data["args"].clear();
      data["args"].push_back(id);
    } else if (data["op"] == "const") {
      data["op"] = "id";
      data["args"].clear();
      data["args"].push_back(id);
      data.erase("value");
    } else {
      // TODO
      dump();
      assert(0);
    }
  }
  void update_args(json args) { data["args"] = args; }

  __attribute__((noinline)) void dump() { cout << "// " <<  data << endl; }
};

class inst_list {
private:
  instruction *head;
  instruction *tail;
  size_t count;
  typedef tuple<string, vector<int>> value;
  // value number
  map<value, int> value2num;
  vector<string> vars;
  map<string, int> var2num;

public:
  inst_list() {
    head = tail = nullptr;
    count = 0;
  }
  instruction *get_last_inst() { return tail; }

  void insert(instruction *inst) {
    if (head && head->is_label()) {
      inst->set_next(head->get_next());
      head->set_next(inst);
    } else {
      inst->set_next(head);
      head = inst;
    }
    count++;
  }

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
  size_t get_count() { return count; }
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

  void get_redefs(set<instruction *> &redefs) {
    instruction *i = head;
    unordered_map<string, instruction *> v2i;
    while (i != nullptr) {
      string dest = i->get_def();
      if (dest != "") {
        if (v2i.find(dest) != v2i.end()) {
          redefs.insert(v2i[dest]);
        }
        v2i[dest] = i;
      }
      i = i->get_next();
    }
    return;
  }

  value get_value(instruction *inst) {
    string dest = inst->get_def();
    string op = inst->get_op();
    vector<string> args = inst->get_args();
    vector<int> iargs;
    if (op == "const") {
      int v = inst->get_data()["value"];
      iargs.push_back(v);
    } else {
      for (auto a : args) {
        if (var2num.find(a) == var2num.end()) {
          vars.push_back(a);
          var2num[a] = vars.size() - 1;
        }
        iargs.push_back(var2num[a]);
      }
      sort(iargs.begin(), iargs.end());
    }
    value owa(op, iargs);
    return owa;
  }

  void get_killed(set<string> &killed) {
    instruction *i = head;
    while (i != nullptr) {
      string def = i->get_def();
      if (def != "")
        killed.insert(def);
      i = i->get_next();
    }
  }

  void erase_by_value(map<string, string> &alias, string name) {
    for (auto it = alias.begin(); it != alias.end();) {
      if (it->second == name) {
        it = alias.erase(it); // erase returns next valid iterator
      } else {
        ++it;
      }
    }
  }

  void lvn() {
    set<instruction *> redefs;
    get_redefs(redefs);
    string lvn = "lvn.";

    map<string, string> alias;

    instruction *i = head;
    while (i != nullptr) {
      value v = get_value(i);

      string dest = i->get_def();
      if (value2num.find(v) != value2num.end()) {
        i->update_to_copy(vars[value2num[v]]);
        if (dest != "")
          var2num[dest] = value2num[v];
      } else {
        string op = i->get_op();
        vector<string> args = i->get_args();
        if (dest != "") {
          if (redefs.count(i) != 0) {
            string new_dest = lvn + to_string(redefs.size());
            redefs.erase(i);
            i->update_def(new_dest);
            vars.push_back(new_dest);
            var2num[new_dest] = vars.size() - 1;
            value2num[v] = vars.size() - 1;
            var2num[dest] = vars.size() - 1;
          } else {
            if (op == "id") {
              string a = args[0];
              while (alias.find(a) != alias.end())
                a = alias[a];
              alias[dest] = a;
            }
            erase_by_value(alias, dest);

            vars.push_back(dest);
            value2num[v] = vars.size() - 1;
            var2num[dest] = vars.size() - 1;
          }
        }
        if (args.size()) {
          json new_args;
          for (auto a : args) {
            string v = vars[var2num[a]];
            if (alias.find(v) != alias.end())
              new_args.push_back(alias[v]);
            else
              new_args.push_back(v);
          }
          i->update_args(new_args);
        }
      }
      i = i->get_next();
    }
  }

  void get_defs(var2instset &defs) {
    instruction *i = head;
    while (i != nullptr) {
      string def = i->get_def();
      if (def != "") {
        set<instruction *> vi;
        vi.insert(i);
        defs[def] = vi;
      }
      i = i->get_next();
    }
  }

  void dump_lvn() {
    cout << "vars: " << endl;
    for (int i = 0; i < vars.size(); ++i)
      cout << (i == 0 ? "" : ",") << vars[i];
    cout << endl;

    cout << "var2num: " << endl;
    for (auto &[v, n] : var2num) {
      cout << v << " : " << n << endl;
    }

    cout << "value2num: " << endl;
    for (auto &[value, n] : value2num) {
      cout << get<0>(value) << " ";
      for (auto i : get<1>(value)) {
        cout << i << " ";
      }
      cout << " : " << n << endl;
    }
    return;
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
