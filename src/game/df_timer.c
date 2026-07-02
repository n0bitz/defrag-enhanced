#include "qagame.h"

DEFINE_HOOK(qboolean, DF_ItemPickupAllowed, (gentity_t* ent))
    (void)ORIGINAL(DF_ItemPickupAllowed);  // intentional reimplementation

    if (!ent->item) {
        return qfalse;
    }
    if (ent->flags & FL_EXPLICIT_GIVE_CMD) {
        return qtrue;
    }

    // DFE change: DeFRaG explicitly prevents personal teleporters from being
    // picked up. Presumably because in fastcaps, using the personal teleporter
    // while the flag is held, results in the flag dropping. Which in turn,
    // would allow for unintended time resets off the dropped flag which may be
    // closer to the player's own base. However, we want the personal teleporter
    // to be usable once again, so we are going to let it be picked up. The
    // issue with flags is taken care of in `ClientEvents`, see that for more
    // info.
    /*
    if (ent->item->giType == IT_HOLDABLE && ent->item->giTag == HI_TELEPORTER) {
        return qfalse;
    }
    */

    // The != 0 is just a nitpick thing I'm adding... We typed the return as
    // qboolean in ReFRaG, but DeFRaG never actually brought the bit test result
    // back to [qfalse, qtrue] (it doesn't matter cause all the existing usages
    // just test with !DF_ItemPickupAllowed(...))
    return (df.itemPickupAllowedBitmap & (1 << ent->item->giType)) != 0;
END_HOOK
