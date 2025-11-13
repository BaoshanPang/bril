#!/usr/bin/env python3

import json
import sys

data = json.load(sys.stdin)
for f in data["functions"]:
    n2b = {}
    blocks = []
    blk = []
    idx = 0
    for i in f["instrs"]:
        idx += 1
        op = i.get("op", "no_op")
        if op in ("br", "jmp"):
            blk.append(i)
            blocks.append(blk)
            blk = []
        elif op == "no_op": # this is a label
            if(len(blk) > 0):
                blocks.append(blk)
            blk = []
            blk.append(i)
            n2b[i["label"]] = blk
        else:
            blk.append(i)
            if(len(blk) == 1):
                n2b["B{}".format(idx)] = blk
    if(len(blk) > 0):
        blocks.append(blk)

    succs = {}
    for i in range(0, len(n2b)):
        name, b = list(n2b.items())[i]
        succs[name] = []
        last = b[len(b) - 1]
        if last["op"] in ("jmp", "br"):
            for l in last["labels"]:
                succs[name].append(l)
        else:
            if i < len(n2b) - 1:
                succs[name].append(list(n2b.keys())[i+1])
    print("digraph {} {{".format(f["name"]))
    for name in n2b:
        print("    {}".format(name))
    for name in succs:
        for l in succs[name]:
            print("    {} -> {}".format(name, l))
    print("}")
