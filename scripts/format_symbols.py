import tomlkit
from argparse import ArgumentParser
from functools import cmp_to_key
from pathlib import Path


def main():
    parser = ArgumentParser()
    parser.add_argument(
        "symbol_files",
        metavar="symbol_file",
        nargs="+",
        type=Path,
        help="symbols file path(s)",
    )
    args = parser.parse_args()

    for symbol_file in args.symbol_files:
        original = symbol_file.read_text()
        formatted = tomlkit.dumps(formatted_doc(tomlkit.loads(original)))
        if original != formatted:
            symbol_file.write_text(formatted)


def formatted_doc(doc):
    formatted_doc = tomlkit.document()
    # sections are ordered by likelihood of changing
    for section in ["code", "hooks", "data"]:
        if table := doc.get(section):
            formatted_doc.append(section, formatted_table(table))
    return formatted_doc


def formatted_table(table):
    formatted_table = tomlkit.table()
    formatted_table.update(sorted(table.items(), key=sort_key))
    for k, v in formatted_table.items():
        formatted_table[k] = formatted_value(v)
    return formatted_table


@cmp_to_key
def sort_key(a, b):
    """
    Symbols are sorted in the following manner:
    - syscalls are after all regular symbols (as they are less likely to change)
    - regular symbols are ordered by ascending offset amongst each other.
    - syscalls are ordered by descending offset amongst each other
       - can be seen as ascending absolute value
       - basically the same as *_syscalls.asm.

    Ascending order is used as it feels natural. Also, the sort is ordered by
    offset rather than symbol name, as it is more stable and avoids changes
    when we rename symbols.
    """
    a, b = a[1], b[1]
    if (a >= 0 and b >= 0) or (a < 0 and b < 0):
        return abs(a) - abs(b)
    return -1 if a >= 0 else 1


def formatted_value(value):
    """
    Regular offsets use hex literals, as that's typically what our tools show.
    Meanwhile, syscalls use decimal literals similar to *_syscalls.asm.
    """
    return tomlkit.value(hex(value) if value >= 0 else str(value))


if __name__ == "__main__":
    main()
