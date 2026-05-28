#include "qagame.h"

DEFINE_HOOK(static void, G_InitGame,
            (int levelTime, int randomSeed, int restart))
    trap_Printf("^0HELLO ^1WORLD ^7(QAGAME)\n");
    ORIGINAL(G_InitGame)(levelTime, randomSeed, restart);
END_HOOK
