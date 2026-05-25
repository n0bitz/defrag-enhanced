import tomllib
from glob import glob
from pathlib import Path
from quatch import Qvm
from uuid import uuid4
from zipfile import ZipFile

# These are used to generate names for the hooked and original functions for
# symbols that'll be hooked. These suffixes are required to be such that they
# generate unique names that never collide with regular code/symbols when
# appended. Just slapping a uuid4 to ensure this is the case *in practice*.
HOOK_SUFFIX = f"_H00K_{uuid4().hex.upper()}"
ORIG_SUFFIX = f"_OR16_{uuid4().hex.upper()}"

COMMON_CFLAGS = [
    "-DDEFRAG",
    f"-DHOOK_SUFFIX={HOOK_SUFFIX}",
    f"-DORIG_SUFFIX={ORIG_SUFFIX}",
]


def patch(qvm_path, init_point, symbols_path, source_glob, includes, cflags=[]):
    with open(symbols_path, "rb") as f:
        symbols_config = tomllib.load(f)

    symbols, to_hook_symbols = process_symbols(symbols_config, init_point)
    qvm = Qvm(qvm_path, symbols)

    output = qvm.add_c_files(
        glob(source_glob), include_dirs=includes, cflags=COMMON_CFLAGS + cflags
    )
    if output:
        raise Exception(f"LCC warnings found:\n{output}")

    unhooked = []
    for symbol in to_hook_symbols:
        # Technically the symbol could have entered qvm.symbols via data not
        # code, but whoever commits such crimes deserves no helpful errors
        # errors and all the issues possible instead...
        if symbol + HOOK_SUFFIX not in qvm.symbols:
            unhooked.append(symbol)
    if unhooked:
        raise Exception(f"Following hook symbol(s) were never hooked: {unhooked}")

    for symbol in to_hook_symbols:
        qvm.replace_calls(symbol + ORIG_SUFFIX, symbol + HOOK_SUFFIX)

    missing_hook_symbols = []
    for symbol in qvm.symbols:
        if symbol.endswith(HOOK_SUFFIX):
            base_symbol = symbol.removesuffix(HOOK_SUFFIX)
            if base_symbol not in to_hook_symbols:
                missing_hook_symbols.append(base_symbol)
    if missing_hook_symbols:
        raise Exception(f"Missing hook symbol(s) for: {missing_hook_symbols}")

    # quatch needs a symbol for init_point, but if it's hooked we hide it...
    # just insert the original init_point symbol for quatch to use now
    # that all the compilation/linking is done
    if init_point in to_hook_symbols:
        qvm.symbols[init_point] = to_hook_symbols[init_point]

    return qvm


def process_symbols(symbols_config, init_point):
    """
    Renames symbols of functions that'll be hooked to force any callers to
    choose which version it wants. Does some validation along the way as it
    makes a merged symbol dict for quatch to use.

    Returns the merged symbol dict for quatch to use and the original hook
    symbol dict to use to do the hooking later.
    """

    def check_addresses_are_unique(symbols):
        value_freqs = {}
        for v in symbols.values():
            value_freqs[v] = value_freqs.get(v, 0) + 1
        if duplicates := {f"{v:#x}" for v, c in value_freqs.items() if c > 1}:
            raise Exception(
                f"Multiple symbols found for these address(es): {duplicates}"
            )

    data_symbols = symbols_config.get("data", {})
    check_addresses_are_unique(data_symbols)

    code_symbols = symbols_config.get("code", {})
    to_hook_symbols = symbols_config.get("hooks", {})
    if init_point not in code_symbols and init_point not in to_hook_symbols:
        raise Exception(f"Symbol for `{init_point}` must be provided")

    code_names, to_hook_names, data_names = (
        code_symbols.keys(),
        to_hook_symbols.keys(),
        data_symbols.keys(),
    )
    duplicate_symbol_names = (
        (code_names & to_hook_names)
        | (code_names & data_names)
        | (to_hook_names & data_names)
    )
    if duplicate_symbol_names:
        raise Exception(f"Duplicate symbol name(s) found: {duplicate_symbol_names}")

    # rename symbols that'll be hooked to hide them from quatch
    symbols = code_symbols | {
        sym + ORIG_SUFFIX: addr for sym, addr in to_hook_symbols.items()
    }
    check_addresses_are_unique(symbols)

    symbols |= data_symbols
    return symbols, to_hook_symbols


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
