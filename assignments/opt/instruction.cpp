#include "instruction.h"
#include "block.h"

void inst_list::ssa_remove_phi(block *b) {
  instruction *i = head;
  while (i != nullptr) {
    if (i->get_op() == "phi") {
      map<string, instruction *> b2i;
      string d = i->get_def();
      json data = i->get_data();
      for (int n = 0; n < data["args"].size(); ++n) {
        string a = data["args"][n];
        string l = data["labels"][n];
        json copy;
        copy["op"] = "id";
        copy["dest"] = d;
        copy["args"].push_back(a);
        instruction *new_inst = new instruction(copy);
        b2i[l] = new_inst;
      }
      for (auto &p : *b->get_preds()) {
        string name = p->get_name();
        if (b2i.find(name) != b2i.end())
          p->append_inst_before_last_jmp(b2i[name]);
      }
      i = unlink_instruction(i);
      continue;
    }
    i = i->get_next();
  }
}
