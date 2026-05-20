import tomllib
from glob import glob
from pathlib import Path
from quatch import Qvm
from zipfile import ZipFile


def patch(in_qvm, symbols, source_glob, includes):
    with open(symbols, "rb") as f:
        symbols = tomllib.load(f)

    qvm = Qvm(in_qvm, symbols=(symbols.get("code", {}) | symbols.get("data", {})))

    output = qvm.add_c_files(
        glob(source_glob), include_dirs=includes, cflags=["-DDEFRAG"]
    )
    if output:
        print(output)

    for symbol in qvm.symbols:
        if symbol.endswith("_H00K"):
            qvm.replace_calls(symbol[: -len("_H00K")], symbol)

    return qvm


build_dir = Path("build/defrag")
build_dir.mkdir(parents=True, exist_ok=True)

print("patching cgame...")
patched_cgame = patch(
    "vm/cgame.qvm",
    "src/cgame/symbols.toml",
    "src/cgame/*.c",
    ["src/cgame", "src/game", "src/sdk/cgame", "src/sdk/game"],
)
patched_cgame.write(build_dir / "cgame.qvm", forge_crc=True)

print("patching game...")
patched_game = patch(
    "vm/qagame.qvm",
    "src/game/symbols.toml",
    "src/game/*.c",
    ["src/game", "src/sdk/game"],
)
patched_game.write(build_dir / "qagame.qvm", forge_crc=True)


print("building pk3...")
with ZipFile(build_dir / "zzzzz-patched-vms.pk3", "w") as pk3:
    pk3.write(build_dir / "cgame.qvm", "vm/cgame.qvm")
    pk3.write(build_dir / "qagame.qvm", "vm/qagame.qvm")
