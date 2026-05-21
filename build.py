import tomllib
from glob import glob
from pathlib import Path
from quatch import Qvm
from zipfile import ZipFile

HOOK_SUFFIX = "_H00K"
ORIG_SUFFIX = "_OR161N4L"


def patch(in_qvm, init_point, symbols, source_glob, includes, cflags=[]):
    with open(symbols, "rb") as f:
        symbols = tomllib.load(f)

    code_symbols = symbols.get("code", {})
    hooked_symbols = symbols.get("hooks", {})

    assert init_point in code_symbols or init_point in hooked_symbols, (
        f"Missing symbol for `{init_point}`"
    )

    duplicate_keys = code_symbols.keys() & hooked_symbols.keys()
    assert len(duplicate_keys) == 0, duplicate_keys

    duplicate_values = set(code_symbols.values()) & set(hooked_symbols.values())
    assert len(duplicate_values) == 0, duplicate_values

    qvm = Qvm(
        in_qvm,
        symbols=(
            code_symbols
            | {(k + ORIG_SUFFIX): v for k, v in hooked_symbols.items()}
            | symbols.get("data", {})
        ),
    )
    output = qvm.add_c_files(
        glob(source_glob), include_dirs=includes, cflags=["-DDEFRAG", *cflags]
    )
    if output:
        print(output)

    # func_addr is used here instead of symbol because we never gave
    # quatch the real symbol name but instead a suffixed version
    for symbol, func_addr in hooked_symbols.items():
        qvm.replace_calls(func_addr, symbol + HOOK_SUFFIX)

    # quatch needs a symbol for init_point, but if it's hooked, we hide it...
    # all the compilation/linking is done, so add it in for quatch to
    # do its thing
    if init_point not in qvm.symbols:
        qvm.symbols[init_point] = qvm.symbols[init_point + ORIG_SUFFIX]

    return qvm


build_dir = Path("build/defrag")
build_dir.mkdir(parents=True, exist_ok=True)

print("patching cgame...")
patched_cgame = patch(
    "vm/cgame.qvm",
    "CG_Init",
    "src/cgame/symbols.toml",
    "src/cgame/*.c",
    ["src/cgame", "src/common", "src/game", "src/sdk/cgame", "src/sdk/game"],
    ["-DCGAME"],
)
patched_cgame.write(build_dir / "cgame.qvm", forge_crc=True)

print("patching game...")
patched_game = patch(
    "vm/qagame.qvm",
    "G_InitGame",
    "src/game/symbols.toml",
    "src/game/*.c",
    ["src/game", "src/common", "src/sdk/game"],
)
patched_game.write(build_dir / "qagame.qvm", forge_crc=True)


print("building pk3...")
with ZipFile(build_dir / "zzzzz-patched-vms.pk3", "w") as pk3:
    pk3.write(build_dir / "cgame.qvm", "vm/cgame.qvm")
    pk3.write(build_dir / "qagame.qvm", "vm/qagame.qvm")
