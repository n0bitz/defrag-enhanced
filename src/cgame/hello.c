#include "cg_local.h"

void CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum);
void CG_Init_H00K(int serverMessageNum, int serverCommandSequence,
                  int clientNum)
{
    trap_Print("^0HELLO ^1WORLD ^7(CGAME)\n");
    CG_Init(serverMessageNum, serverCommandSequence, clientNum);
}
