#include "g_local.h"

void G_InitGame(int levelTime, int randomSeed, int restart);
void G_InitGame_H00K(int levelTime, int randomSeed, int restart)
{
    trap_Printf("^0HELLO ^1WORLD ^7(QAGAME)\n");
    G_InitGame(levelTime, randomSeed, restart);
}
