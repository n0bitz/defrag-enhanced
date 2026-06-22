#include "cgame.h"

// See instruction_patches.py for usage.
extern qboolean DF_UpdateTimerAndCheckpoints_ShouldDoFinishStuff(void)
{
    return !sv_cheats && cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR;
}
