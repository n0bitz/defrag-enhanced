#include "cgame.h"

static void CG_DrawItemBBox(centity_t* cent)
{
    playerState_t* ps;
    vec3_t mins, maxs;
    entityState_t* es = &cent->currentState;
    static byte color[4] = {0, 255, 0, 255};

    if (!es->modelindex || (es->eFlags & EF_NODRAW)) return;

    ps = &cg.snap->ps;

    // Skip drawing the bbox if df won't show the item in some way shape or
    // form. We'll still draw the bbox if the item is shown with the not
    // available shader.
    if (
       (cent->nextState.eType == ET_ITEM &&
        !IsItemEntityAvailableToClient(&cent->nextState, ps->clientNum)) ||
       !IsItemEntityAvailableToClient(&cent->currentState, ps->clientNum)
    ) {
        qboolean is_flag;

        // What is df even doing? Why is multiplayer special?
        // Also hate how many options df_cl_alwaysDrawItems has...
        // Why is this so unnecessarily overcomplicated?
        if (!defragInfo.is_multiplayer || !df_cl_alwaysDrawItems.integer) {
            return;
        }

        is_flag = bg_itemlist[es->modelindex].giType == IT_TEAM;
        if (cg_simpleItems.integer && !is_flag) {
            if (df_cl_alwaysDrawItems.integer == 2) return;
        } else if (df_cl_alwaysDrawItems.integer == 3) {
            return;
        }
    }

    // total mins/maxs from BG_PlayerTouchesItem
    VectorSet(mins, -50.0f, -36.0f, -36.0f);
    VectorSet(maxs, 44.0f, 36.0f, (ps->pm_flags & PMF_PROMODE) ? 66.0f : 36.0f);

    // account for player's bbox (adapted from PM_CheckDuck)
    maxs[0] += -15;
    maxs[1] += -15;

    mins[0] += 15;
    mins[1] += 15;

    maxs[2] += MINS_Z;
    mins[2] += (ps->pm_flags & PMF_DUCKED) ? 16 : 32;

    CG_DrawBoundingBox(cent->currentState.origin, mins, maxs, color);
}

DEFINE_HOOK(void, CG_Item, (centity_t* cent))
    ORIGINAL(CG_Item)(cent);

    if (cg_drawBBox.integer) {
        CG_DrawItemBBox(cent);
    }
END_HOOK

/*
===============
CG_AddCEntity
===============
*/
DEFINE_HOOK(void, CG_AddCEntity, (centity_t* cent))
    CG_AddCEntityPOI(cent);
    ORIGINAL(CG_AddCEntity)(cent);
END_HOOK
