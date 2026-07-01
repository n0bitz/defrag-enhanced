# WARNING: Please avoid using instruction level patches if possible. These
# should only be used when the alternatives are too painful, unstable or hard
# to understand.
from quatch import Qvm, Instruction as I, Opcode as O
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    patch_func: callable = ...


@patch_func
def DF_ItemPickupAllowed(qvm: Qvm):
    func = "DF_ItemPickupAllowed"
    func_addr = qvm.symbols[func]

    # To let personal teleporters be picked up, the following is done:
    # ```diff
    #     if (ent->flags & FL_EXPLICIT_GIVE_CMD) {
    #         return qtrue;
    #     }
    # -    if (ent->item->giType == IT_HOLDABLE && ent->item->giTag == HI_TELEPORTER) {
    # -        return qfalse;
    # -    }
    #     return df.itemPickupAllowedBitmap & (1 << ent->item->giType);
    # }
    # ```
    # Fastcaps breaking when you use the personal teleporter is handled with a
    # `ClientEvents` instruction patch. See that for more information on why DF
    # may have prevented it from being picked up.
    qvm.replace_instructions(
        func_addr + 0x19,
        [
            I(O.LOCAL, 0x8),
            I(O.LOCAL, 0x14),
            I(O.LOAD4),
            I(O.CONST, 0x324),
            I(O.ADD),
            I(O.LOAD4),
            I(O.STORE4),
            I(O.LOCAL, 0x8),
            I(O.LOAD4),
            I(O.CONST, 0x24),
            I(O.ADD),
            I(O.LOAD4),
            I(O.CONST, 0x6),
            I(O.NE, func_addr + 0x32),
            I(O.LOCAL, 0x8),
            I(O.LOAD4),
            I(O.CONST, 0x28),
            I(O.ADD),
            I(O.LOAD4),
            I(O.CONST, 0x1),
            I(O.NE, func_addr + 0x32),
            I(O.CONST, 0x0),
            I(O.LEAVE, 0xC),
            I(O.CONST, func_addr + 0x40),
            I(O.JUMP),
        ],
        [I(O.UNDEF)] * 25,
    )


@patch_func
def ClientEvents(qvm: Qvm):
    func = "ClientEvents"
    func_addr = qvm.symbols[func]

    # When personal teleporters are used, the flag drops. This is a problem in
    # fastcaps as that lets you time reset off the dropped flag, which may be
    # in a more advantageous position (ie. closer to one's own base).
    # I suspect DF explicitly prevented personal teleporters from being picked
    # up for this reason. Since we now allow personal teleporeters to be picked
    # up, the following is done to remove the flag from the player without
    # actually dropping it when personal teleporters are used:
    # ```diff
    # case EV_USE_ITEM1: // teleporter
    #     ...
    #     if ( item ) {
    # -       drop = Drop_Item( ent, item, 0 );
    # -       // decide how many seconds it has left
    # -       drop->count = ( ent->client->ps.powerups[ j ] - level.time ) / 1000;
    # -       if ( drop->count < 1 ) {
    # -           drop->count = 1;
    # -       }
    # -
    #         ent->client->ps.powerups[ j ] = 0;
    #     }
    # ```
    qvm.replace_instructions(
        func_addr + 0xF6,
        [
            I(O.LOCAL, 0x90),
            I(O.LOAD4),
            I(O.ARG, 0x8),
            I(O.LOCAL, 0x34),
            I(O.LOAD4),
            I(O.ARG, 0xC),
            I(O.CONST, 0x0),
            I(O.ARG, 0x10),
            I(O.LOCAL, 0x80),
            I(O.CONST, qvm.symbols["Drop_Item"]),
            I(O.CALL),
            I(O.STORE4),
            I(O.LOCAL, 0x64),
            I(O.LOCAL, 0x80),
            I(O.LOAD4),
            I(O.STORE4),
            I(O.LOCAL, 0x64),
            I(O.LOAD4),
            I(O.CONST, 0x2F8),
            I(O.ADD),
            I(O.LOCAL, 0x44),
            I(O.LOAD4),
            I(O.CONST, 0x2),
            I(O.LSH),
            I(O.LOCAL, 0x90),
            I(O.LOAD4),
            I(O.CONST, 0x204),
            I(O.ADD),
            I(O.LOAD4),
            I(O.CONST, 0x138),
            I(O.ADD),
            I(O.ADD),
            I(O.LOAD4),
            I(O.CONST, 0x12A310),
            I(O.LOAD4),
            I(O.SUB),
            I(O.CONST, 0x3E8),
            I(O.DIVI),
            I(O.STORE4),
            I(O.LOCAL, 0x64),
            I(O.LOAD4),
            I(O.CONST, 0x2F8),
            I(O.ADD),
            I(O.LOAD4),
            I(O.CONST, 0x1),
            I(O.GEI, func_addr + 0x12A),
            I(O.LOCAL, 0x64),
            I(O.LOAD4),
            I(O.CONST, 0x2F8),
            I(O.ADD),
            I(O.CONST, 0x1),
            I(O.STORE4),
        ],
        [I(O.UNDEF)] * 52,
    )
