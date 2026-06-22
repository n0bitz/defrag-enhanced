# WARNING: Please avoid using instruction level patches if possible. These
# should only be used when the alternatives are too painful, unstable or hard
# to understand.
from quatch import Qvm, Instruction as I, Opcode as O
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    patch_func: callable = ...


@patch_func
def DF_UpdateTimerAndCheckpoints(qvm: Qvm):
    func = "DF_UpdateTimerAndCheckpoints"
    func_addr = qvm.symbols[func]

    # nop the cheats check for regular checkpoint events so we can see
    # checkpoint prints when playing with cheats
    qvm.replace_instructions(
        func_addr + 0x71,
        [
            I(O.CONST, qvm.symbols["sv_cheats"]),
            I(O.LOAD4),
            I(O.CONST, 0x0),
            I(O.NE, func_addr + 0x9F),
        ],
        [I(O.UNDEF)] * 4,
    )

    # nop the cheats check for finish checkpoint event so we can see the print
    # when playing with cheats
    qvm.replace_instructions(
        func_addr + 0x9F,
        [
            I(O.LOCAL, 0x138),
            I(O.CONST, 0x0),
            I(O.STORE4),
            I(O.CONST, qvm.symbols["sv_cheats"]),
            I(O.LOAD4),
            I(O.LOCAL, 0x138),
            I(O.LOAD4),
            I(O.NE, func_addr + 0x15A),
        ],
        [I(O.LOCAL, 0x138), I(O.CONST, 0x0), I(O.STORE4)] + [I(O.UNDEF)] * 5,
    )

    # Since we nop-ed the cheats check for finish checkpoint events earlier,
    # we have to skip over the record saving and global bitflag logic stuff
    # that autorecord and stuff use. As, it makes no sense to have cheated
    # runs overwrite pbs and such.
    qvm.replace_instructions(
        func_addr + 0xB6,
        [
            I(O.CONST, 0x6C0),
            I(O.CONST, 0xCFCCC),
            I(O.LOAD4),
            I(O.MULI),
            I(O.CONST, 0xF87C0),
            I(O.ADD),
            I(O.LOAD4),
            I(O.CONST, 0x3),
            I(O.EQ, func_addr + 0x154),
        ],
        [
            I(O.CONST, qvm.symbols[func + "_ShouldDoFinishStuff"]),
            I(O.CALL),
            I(O.CONST, 0x0),
            I(O.EQ, func_addr + 0x154),
        ]
        + [I(O.UNDEF)] * 5,
    )
