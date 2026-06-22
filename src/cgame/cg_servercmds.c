#include "cgame.h"

DEFINE_HOOK(void, CG_ConfigStringModified, (void))
    ORIGINAL(CG_ConfigStringModified)();

    // This is to fix stale checkpoints being loaded/saved to when
    // physics/gametype changes. Reloading pb/session checkpoints when
    // serverinfo configstrings change similar to how .27 did.
    if (atoi(CG_Argv(1)) == CS_SERVERINFO) {
        DF_LoadBestCheckpoints();
        DF_LoadSessionCheckpoints();
    }
END_HOOK
