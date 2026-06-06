import json
import importlib
import tomllib
from argparse import ArgumentParser, ArgumentDefaultsHelpFormatter
from dataclasses import dataclass, KW_ONLY, field, InitVar
from glob import glob
from pathlib import Path
from quatch import Qvm
from uuid import uuid4
from zipfile import ZipFile


def main():
    COMMON_SOURCES = (
        list(glob("src/common/*.c"))
        + list(glob("src/libc/*.c"))
        + ["src/third_party/tlsf/tlsf.c"]
    )
    PROJECTS = [
        Project(
            name="cgame",
            init_point="CG_Init",
            symbols_path="src/cgame/symbols.toml",
            source_files=list(glob("src/cgame/*.c")) + COMMON_SOURCES,
            include_paths=[
                "src/cgame",
                "src/common",
                "src/game",
                "src/sdk/cgame",
                "src/sdk/game",
                "src/libc",
                "src/third_party",
            ],
            instruction_patch_files=["src/cgame/instruction_patches.py"],
            extra_defines=["CGAME"],
        ),
        Project(
            name="qagame",
            init_point="G_InitGame",
            symbols_path="src/game/symbols.toml",
            source_files=list(glob("src/game/*.c")) + COMMON_SOURCES,
            include_paths=[
                "src/game",
                "src/common",
                "src/sdk/game",
                "src/libc",
                "src/third_party",
            ],
        ),
        Project(
            name="ui",
            init_point="UI_Init",
            symbols_path="src/ui/symbols.toml",
            source_files=list(glob("src/ui/*.c")) + COMMON_SOURCES,
            include_paths=[
                "src/ui",
                "src/common",
                "src/sdk/q3_ui",
                "src/sdk/game",
                "src/libc",
                "src/third_party",
            ],
        ),
    ]

    STEPS = [
        COMPILE_COMMANDS_STEP := "compile-commands",
        PK3_STEP := "pk3",
    ]
    parser = ArgumentParser(formatter_class=ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        "steps",
        nargs="*",
        choices=STEPS,
        default=STEPS,
        metavar="step",
        help="step(s) to run",
    )
    args = parser.parse_args()

    if COMPILE_COMMANDS_STEP in args.steps:
        generate_compile_commands(PROJECTS)

    if PK3_STEP in args.steps:
        build(PROJECTS)


@dataclass(frozen=True)
class Project:
    _: KW_ONLY
    # Base name of the VM (eg. "qagame").
    name: str
    init_point: str
    symbols_path: str
    source_files: list[str]
    include_paths: list[str]
    # WARNING: Please avoid using instruction level patches if possible. These
    # should only be used when the alternatives are too painful, unstable or
    # hard to understand.
    instruction_patch_files: list[str] | None = None
    # Nothing besides cflags should need this, so `InitVar` to hide it in an
    # internal field in `__post_init__`. Also not  `= []` as it would be a
    # shared mutable default.
    extra_defines: InitVar[list[str] | None] = None
    _extra_defines: list[str] = field(init=False)
    # These are used to generate names for the hooked and original functions for
    # symbols that'll be hooked. These suffixes are required to be such that they
    # generate unique names that never collide with regular code/symbols when
    # appended. Just slapping a uuid4 to ensure this is the case *in practice*.
    hook_suffix: str = field(init=False)
    orig_suffix: str = field(init=False)

    def cflags(self, for_build):
        cflags = [f"-D{define}" for define in ["DEFRAG", *self._extra_defines]]

        # This is a hack to prevent bg_lib.h from being included by q_shared.h since it
        # conflicts with our libc implementation.
        cflags.append("-UQ3_VM")

        if for_build:
            cflags.append(f"-DHOOK_SUFFIX={self.hook_suffix}")
            cflags.append(f"-DORIG_SUFFIX={self.orig_suffix}")
            cflags.extend(f"-I{include}" for include in self.include_paths)
        else:
            # Make clang behave similar to LCC (ie. ANSI C + single line
            # comments + 32 bits) and treats warnings as errors like we do
            # for quatch compiled code.
            # NOTE: It unfortunately still diverges from LCC on some things
            # (eg. sizeof(double) is 8 in clang and 4 in LCC), but there
            # seems to be no way at present to make it work besides forking
            # llvm/clang. It shouldn't be a problem for the most part though
            # as no one should be using such types to begin with.
            # NOTE: This section is just to emulate LCC, keep any additional
            # warning or diagnostic flags in .clang-tidy instead.
            cflags.extend(
                (
                    "-ansi",
                    "-pedantic-errors",
                    "-Wno-comment",
                    "-Werror",
                    "-m32",
                )
            )

            # It isn't a valid attribute in ANSI C, while some of the sdk
            # headers use it gated behind platform ifdefs. Our linting has some
            # of those platform defines set causing bail outs after several
            # unnecessary diagnostics... Just noop it to suppress those errors,
            # rather than finding and undef-ing the offending platform defines
            # since inline is eventually a reserved attribute anyway.
            cflags.append("-Dinline=")

            # So we can do some ifdef macro hacks to generate additional diagnostics
            # that noop in real build
            cflags.append("-DLINTER")

            for include in self.include_paths:
                if include.startswith("src/sdk") or include.startswith(
                    "src/third_party"
                ):
                    # to prevent diagnosing external stuff while still including them
                    cflags.extend(("-isystem", include))
                else:
                    cflags.append(f"-I{include}")

        return cflags

    def __post_init__(self, extra_defines):
        object.__setattr__(self, "hook_suffix", f"_H00K_{uuid4().hex.upper()}")
        object.__setattr__(self, "orig_suffix", f"_OR16_{uuid4().hex.upper()}")
        object.__setattr__(self, "_extra_defines", extra_defines or [])


def generate_compile_commands(projects: list[Project]):
    print("INFO: Building compilation database...")
    compile_commands = []
    root_dir = str(Path(__file__).resolve().parent)
    for project in projects:
        for file in project.source_files:
            arguments = ["clang", *project.cflags(for_build=False), file]
            compile_commands.append(
                {"directory": root_dir, "file": file, "arguments": arguments}
            )

    with open("compile_commands.json", "w") as f:
        json.dump(compile_commands, f)


def build(projects: list[Project]):
    build_dir = Path("build/defrag")
    build_dir.mkdir(parents=True, exist_ok=True)
    for project in projects:
        print(f"INFO: Building {project.name}...")
        patched_vm = patch(project)
        patched_vm.write(build_dir / f"{project.name}.qvm", forge_crc=True)

    print("INFO: Building pk3...")
    with ZipFile(build_dir / "zzzzz-defrag-enhanced.pk3", "w") as pk3:
        for asset in Path("assets").rglob("*"):
            pk3.write(asset, f"{asset.relative_to('assets')}")
        for project in projects:
            qvm_file_name = f"{project.name}.qvm"
            pk3.write(build_dir / qvm_file_name, f"vm/{qvm_file_name}")


def filter_warnings(output: str):
    ignores = [
        "expression with no effect elided",
        "overflow in converting constant expression from `int' to `unsigned int'",
    ]

    def ignored(line):
        return any(msg in line for msg in ignores)

    return "\n".join(line for line in output.splitlines() if not ignored(line))


def patch(project: Project):
    symbols, to_hook_symbols = process_symbols(project)
    qvm = Qvm(f"vm/{project.name}.qvm", symbols)

    output = qvm.add_c_files(
        project.source_files, project.include_paths, project.cflags(for_build=True)
    )
    if output and (warnings := filter_warnings(output)):
        raise Exception(f"LCC warnings found:\n{warnings}")

    hook_suffix, orig_suffix = project.hook_suffix, project.orig_suffix
    unhooked = []
    for symbol in to_hook_symbols:
        # Technically the symbol could have entered qvm.symbols via data not
        # code, but whoever commits such crimes deserves no helpful errors
        # errors and all the issues possible instead...
        if symbol + hook_suffix not in qvm.symbols:
            unhooked.append(symbol)
    if unhooked:
        raise Exception(f"Following hook symbol(s) were never hooked: {unhooked}")

    for symbol in to_hook_symbols:
        qvm.replace_calls(symbol + orig_suffix, symbol + hook_suffix)

    missing_hook_symbols = []
    for symbol in qvm.symbols:
        if symbol.endswith(hook_suffix):
            base_symbol = symbol.removesuffix(hook_suffix)
            if base_symbol not in to_hook_symbols:
                missing_hook_symbols.append(base_symbol)
    if missing_hook_symbols:
        raise Exception(f"Missing hook symbol(s) for: {missing_hook_symbols}")

    if redefined_symbols := qvm.symbols.keys() & to_hook_symbols.keys():
        raise Exception(f"Following hook symbol(s) were redefined: {redefined_symbols}")

    # now that all the compilation/linking is done, bring the original symbols
    # that we hid back in for quatch and instruction level patches to use
    qvm.symbols |= to_hook_symbols

    def patch_func_decorator(f):
        f(qvm)
        return f

    for patch_file in project.instruction_patch_files or []:
        spec = importlib.util.spec_from_file_location(patch_file, patch_file)
        patcher = importlib.util.module_from_spec(spec)
        patcher.__dict__["patch_func"] = patch_func_decorator
        spec.loader.exec_module(patcher)

    return qvm


def process_symbols(project: Project):
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

    with open(project.symbols_path, "rb") as f:
        symbols_config = tomllib.load(f)

    data_symbols = symbols_config.get("data", {})
    check_addresses_are_unique(data_symbols)

    code_symbols = symbols_config.get("code", {})
    to_hook_symbols = symbols_config.get("hooks", {})
    init_point = project.init_point
    if init_point not in code_symbols and init_point not in to_hook_symbols:
        raise Exception(f"Symbol for `{init_point}` must be provided")

    duplicate_symbol_names = (
        (code_symbols.keys() & to_hook_symbols.keys())
        | (code_symbols.keys() & data_symbols.keys())
        | (to_hook_symbols.keys() & data_symbols.keys())
    )
    if duplicate_symbol_names:
        raise Exception(f"Duplicate symbol name(s) found: {duplicate_symbol_names}")

    # rename symbols that'll be hooked to hide them from quatch
    symbols = code_symbols | {
        sym + project.orig_suffix: addr for sym, addr in to_hook_symbols.items()
    }
    check_addresses_are_unique(symbols)

    symbols |= data_symbols
    return symbols, to_hook_symbols


if __name__ == "__main__":
    main()
