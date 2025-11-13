#include <string>
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

class block {
private:
  std::string name;
  std::vector<json> instructions;

public:
  block() { name = "unamed"; }
  void set_name(int idx) {
    if (instructions[0].contains("label")) {
      name = instructions[0]["label"];
    } else {
      name = "B";
      name += to_string(idx);
    }
  }
  string get_name() { return name; }
  void append_instruction(json inst) { instructions.push_back(inst); }
  json get_last_instruction() { return instructions[size()-1];}
  size_t size() { return instructions.size();}
  void dump() {
    cout << "block: " << name << endl;
    for (auto inst : instructions)
      cout << inst << endl;
  }
};

int main() {
  json j;
  std::cin >> j;
  //std::cout << j.dump(4);
  for (auto func : j["functions"]) {
    std::vector<block*> blocks;
    std::map<std::string, block*> name2block;
    block *b = new block;
    for (auto inst : func["instrs"]) {
      // std::cout << inst << std::endl;
      if (inst.contains("op")) {
        b->append_instruction(inst);
        if (inst["op"] == "jmp" || inst["op"] == "br") {
          blocks.push_back(b);
          b = new block;
        }
      } else { // label
        if (b->size())
          blocks.push_back(b);
        b = new block;
        b->append_instruction(inst);
      }
    }
    if (b->size())
      blocks.push_back(b);
    else
      free(b);
#if 0
    for (auto b : blocks) {
      b->dump();
    }
#endif
    int i = 0;
    for (auto blk : blocks) {
      blk->set_name(i);
      name2block[blk->get_name()] = blk;
      i++;
    }
#if 0
    for (auto &[name, blk] : name2block) {
      std::cout << name << std::endl;
      blk->dump();
    }
#endif
#if 1
    // CFG
    std::map<block *, vector<block*>*> cfg;
    i = 0;
    for (auto &[name, blk] : name2block) {
      auto inst = blk->get_last_instruction();
      if (inst["op"] == "br" || inst["op"] == "jmp") {
        vector<block*> *blks = new vector<block*>();
        for (auto l : inst["labels"]) {
          blks->push_back(name2block[l]);
        }
        cfg[blk] = blks;
      } else {
        if (i < name2block.size() - 1) {
          vector<block*> *blks = new vector<block*>;
          blks->push_back(name2block[std::string("B") + std::to_string(i)]);
          cfg[blk] = blks;
        }
      }
      i++;
    }
    cout << "digraph {} {" << endl;
    for (auto [name, blk] : name2block) {
      cout << "    " << name << endl;
    }
    for (auto &[b, succs] : cfg) {
      for (auto s : *succs) {
        if(s)
          cout << "    " << b->get_name() << " -> " << s->get_name() << endl;
      }
    }
    cout << "}" << endl;
#endif
  }


  return 0;
}
