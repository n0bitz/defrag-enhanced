#include "cgame.h"

consoleCommandStatus_t CG_SaveState_f(void)
{
    char cvar_name[MAX_TOKEN_CHARS];
    char restore_command[MAX_STRING_CHARS];
    saveState_t state;

    // TODO: defrag doesn't, but maybe we should validate the chars of
    // output_cvar_name (eg. no spaces and stuff...)
    trap_Argv(1, cvar_name, sizeof(cvar_name));
    if (cvar_name[0] == '\0') {
        Com_Printf("Usage: /%s <cvar name>\n", CG_Argv(0));
        return CON_CMD_HANDLED;
    }

    if (!SaveCurrentState(&state)) {
        trap_Print("^1ERROR: Saving current state is not supported\n");
        return CON_CMD_HANDLED;
    }

    // TODO: LINTER_ASSERT that everything will fit into sizeof(restore_command)
    // somewhere
    Q_strncpyz(restore_command, RESTORE_COMMAND " ", sizeof(restore_command));
    SerializeSaveState(&state,
                       restore_command + sizeof(RESTORE_COMMAND " ") - 1);
    trap_Cvar_Set(cvar_name, restore_command);

    trap_Print("^2Saved\n");
    return CON_CMD_HANDLED;
}

// Command interceptor to restore some stuff client-side before forwarding the
// command along to the server. Ideally the server would handle everything, and
// nothing needs to be done here, but some stuff stuff is just inherently
// client-side like the checkpoint history...
consoleCommandStatus_t CG_RestoreState_f(void)
{
    char serialized_state[MAX_TOKEN_CHARS];
    saveState_t state;

    // NOTE: The server does more validation than here and can eventually reject
    // the command while we already change stuff here anyway. While we can come
    // up with some fancy design to keep them synced somehow, it's not really
    // that important. Like, yeah you might switch weapons or restore cp while
    // the server doesn't restore the rest. But, so what? What is important is
    // that we don't try to restore stuff client side when cheats are off, lest
    // it break normal gameplay somehow.
    if (!sv_cheats) {
        trap_Print("^1ERROR: Cheats are not enabled on this server\n");
        return CON_CMD_HANDLED;
    }

    trap_Argv(1, serialized_state, sizeof(serialized_state));
    if (serialized_state[0] == '\0') {
        Com_Printf("Usage: /%s <serialized save state>\n", CG_Argv(0));
        return CON_CMD_HANDLED;
    }

    if (!DeserializeSaveState(serialized_state, &state)) {
        // TODO: Our own print with levels and stuff mb?
        trap_Print("^1ERROR: Corrupted/invalid savestate\n");
        return CON_CMD_HANDLED;
    }

    // If the currently equipped weapon is different from the one in the
    // savestate, the server will think the client is trying to weapon change
    // away from the one in the savestate. This is because, the server processes
    // ucmds after the restorestate command (see SV_ExecuteClientMessage), and
    // thinks the client wants to switch to the current weapon instead. That's
    // the exact opposite of what we want, so to prevent it, patch the weapon
    // in the current ucmd to be consistent with the savestate.
    cg.weaponSelect = state.weapon;
    cg.weaponSelectTime = cg.time;  // probably isn't needed
    // The ucmd set technically isn't needed as engines run console commands
    // before draw active where cg.weaponSelect is ucmd set.
    trap_SetUserCmdValue(cg.weaponSelect, cg.zoomSensitivity);

    // let it go through so the server can do the actual restoring
    return CON_CMD_NOT_HANDLED;
}

// This function is kinda fragile in that if defrag ever changes the
// implementation of the underlying UpdateTimer to add/remove side-effects, we
// might end up doing unintended stuff. I doubt defrag would ever change though,
// so let's not care too much, since the alternative would be to reimplement the
// decryption logic ourselves.
static int GetTimerTime(snapshot_t* snap)
{
    int tmp = timer_time;
    int time = UpdateTimer(snap, NULL);

    timer_time = tmp;
    return time;
}

qboolean SaveCurrentState(saveState_t* out)
{
    playerState_t* ps = &cg.snap->ps;

    if (ps->pm_type != PM_NORMAL) {
        return qfalse;
    }

    out->pm_flags = ps->pm_flags;
    out->pm_time = ps->pm_time;
    VectorCopy(ps->origin, out->origin);
    VectorCopy(ps->velocity, out->velocity);
    out->weaponTime = ps->weaponTime;
    VectorCopy(ps->viewangles, out->viewangles);
    out->health = ps->stats[STAT_HEALTH];
    out->holdable = ps->stats[STAT_HOLDABLE_ITEM];
    out->weapons = ps->stats[STAT_WEAPONS];
    out->armor = ps->stats[STAT_ARMOR];
    out->jump_time = ps->stats[STAT_JUMPTIME];
    out->djing = ps->stats[STAT_DJING];
    out->score = ps->persistant[PERS_SCORE];
    memcpy(out->powerups, ps->powerups, sizeof(ps->ammo));
    memcpy(out->ammo, ps->ammo, sizeof(ps->ammo));
    out->weapon = ps->weapon;
    out->weaponstate = ps->weaponstate;
    out->serverTime = cg.snap->serverTime;
    out->timer_time = GetTimerTime(cg.snap);
    // TODO: upstream a const for 2
    out->timer_running = !!(ps->stats[STAT_MISC] & 2);

    return qtrue;
}
