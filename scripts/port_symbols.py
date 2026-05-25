import argparse
import hashlib
from dataclasses import dataclass
from pathlib import Path

import quatch
import tomlkit
from quatch import Instruction, Opcode


@dataclass
class Function:
    index: int
    address: int
    op_hash: str | None
    code: list[Instruction]


class Qvm:
    def __init__(self, path):
        qvm = quatch.Qvm(path)

        self.functions = []
        for address, instruction in enumerate(qvm.instructions):
            if instruction.opcode == Opcode.ENTER:
                self.functions.append(Function(len(self.functions), address, None, []))
            self.functions[-1].code.append(instruction)

        self.by_address = {f.address: f for f in self.functions}

        self.by_op_hash = {}
        for f in self.functions:
            opcodes = bytearray(ins.opcode for ins in f.code)
            f.op_hash = hashlib.sha256(opcodes).hexdigest()
            if f.op_hash not in self.by_op_hash:
                self.by_op_hash[f.op_hash] = []
            self.by_op_hash[f.op_hash].append(f)


def match_functions(a: Qvm, b: Qvm):
    matches = {0: 0}  # vmMain is always at 0

    for op_hash, a_funcs in a.by_op_hash.items():
        b_funcs = b.by_op_hash.get(op_hash, [])
        if len(a_funcs) != len(b_funcs):
            continue
        for f, g in zip(a_funcs, b_funcs):
            matches[f.address] = g.address

    # For each pair of nonconsecutive matched functions in A, see if there is a
    # corresponding pair in B with the same amount of nonmatching functions in between.
    # If there is, assume the nonmatching functions match each other.
    a_matches, b_matches = list(matches.keys()), list(matches.values())
    for i in range(len(matches) - 1):
        fs = a.by_address[a_matches[i]], a.by_address[a_matches[i + 1]]
        gs = b.by_address[b_matches[i]], b.by_address[b_matches[i + 1]]

        # skip consecutive matches
        if fs[0].index + 1 == fs[1].index:
            continue

        # skip gaps with different sizes
        gap = fs[1].index - fs[0].index
        if gap != gs[1].index - gs[0].index:
            continue

        for j in range(gap):
            f = a.functions[fs[0].index + j]
            g = b.functions[gs[0].index + j]
            matches[f.address] = g.address

    return matches


def match_data(
    a: Qvm,
    b: Qvm,
    a_addrs: list[int],
    function_matches: dict[int, int],
):
    # TODO this is complete garbage and will break
    matches = {}
    for a_addr in a_addrs:
        b_addr = None
        for f in (a.by_address[addr] for addr in function_matches.keys()):
            for offset, ins in enumerate(f.code):
                if ins.opcode == Opcode.CONST and ins.operand == a_addr:
                    g = b.by_address[function_matches[f.address]]
                    if b_addr is None and len(g.code) > offset:
                        b_addr = g.code[offset].operand
                        matches[a_addr] = b_addr
    return matches


def port_symbols(a: Qvm, b: Qvm, symbols: tomlkit.TOMLDocument):
    function_matches = match_functions(a, b)
    data_matches = match_data(a, b, symbols["data"].values(), function_matches)

    for name, addr in symbols["code"].items():
        if addr > 0:
            symbols["code"][name] = tomlkit.value(hex(function_matches[addr]))

    for name, addr in symbols["hooks"].items():
        if addr > 0:
            symbols["hooks"][name] = tomlkit.value(hex(function_matches[addr]))

    for name, addr in symbols["data"].items():
        symbols["data"][name] = tomlkit.value(hex(data_matches[addr]))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("old", type=Path)
    parser.add_argument("new", type=Path)
    parser.add_argument("symbols", type=Path)
    args = parser.parse_args()

    a = Qvm(args.old)
    b = Qvm(args.new)
    symbols = tomlkit.loads(args.symbols.read_text())

    port_symbols(a, b, symbols)
    print(tomlkit.dumps(symbols))


if __name__ == "__main__":
    main()
