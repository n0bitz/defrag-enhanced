#include "cgame.h"

DEFINE_HOOK(void CG_Init,
            (int serverMessageNum, int serverCommandSequence, int clientNum))
{
    trap_Print("^0HELLO ^1WORLD ^7(CGAME)\n");
    ORIGINAL(CG_Init)(serverMessageNum, serverCommandSequence, clientNum);
}
