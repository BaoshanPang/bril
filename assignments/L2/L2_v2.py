#!/usr/bin/env python3

import json
import sys

def formal_block(insts):
    block = []
    for inst in insts:
        if 'op' in inst:
            block.append(inst)
            if inst["op"] in ("br","jmp"):
                yield block
                block = []
        else:
            if len(block) > 0:
                yield block
            block = [inst]
    yield block



def mycfg():
    prog = json.load(sys.stdin)
    for func in prog["functions"]:
        for blk in formal_block(func["instrs"]):
            print(blk)


if __name__ == "__main__":
    mycfg()
