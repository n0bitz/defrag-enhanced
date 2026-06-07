#include "qagame.h"

void DF_ResetClientWaits(int clientNum);
static void RestoreState(gentity_t* ent, saveState_t* state)
{
    int i;
    playerState_t* ps = &ent->client->ps;
    vec3_t origin, viewangles, velocity;

    ps->pm_flags = state->pm_flags & ~PMF_FOLLOW;
    ps->pm_time = state->pm_time;
    VectorCopy(state->origin, origin);
    VectorCopy(state->velocity, velocity);
    ps->weaponTime = state->weaponTime;
    VectorCopy(state->viewangles, viewangles);
    ent->health = state->health;
    ps->stats[STAT_HEALTH] = state->health;
    ps->stats[STAT_HOLDABLE_ITEM] = state->holdable;
    ps->stats[STAT_WEAPONS] = state->weapons;
    ps->stats[STAT_ARMOR] = state->armor;
    ps->stats[STAT_JUMPTIME] = state->jump_time;
    ps->stats[STAT_DJING] = state->djing;
    ps->persistant[PERS_SCORE] = state->score;
    for (i = 0; i < MAX_POWERUPS; i++) {
        ps->powerups[i] = state->powerups[i];
        if (ps->powerups[i] == 0) continue;
        if (i == PW_REDFLAG || i == PW_BLUEFLAG) {
            ps->powerups[i] = INT_MAX;
        } else {
            ps->powerups[i] += level.time - state->serverTime;
        }
    }
    memcpy(ps->ammo, state->ammo, sizeof(ps->ammo));
    ps->weapon = state->weapon;
    ps->weaponstate = state->weaponstate;
    timers[ent - g_entities].time = state->timer_time;
    timers[ent - g_entities].timer_running = state->timer_running;
    timers[ent - g_entities].num_checkpoints = state->num_checkpoints_hit;
    timers[ent - g_entities].checkpoint_bitmap = 0;

    DF_ResetClientWaits(ent - g_entities);
    DF_PlacePlayerTeleport(ent, origin, viewangles, velocity);
}

void Cmd_RestoreState_f(gentity_t* ent)
{
    char stringifed_state[MAX_TOKEN_CHARS];
    saveState_t state;
    playerState_t* ps = &ent->client->ps;

    if (!trap_Cvar_VariableIntegerValue("sv_cheats")) {
        trap_SendServerCommand(ent - g_entities,
                               "print \"" LOG_ERROR
                               "Cheats are not enabled on this server\n\"");
        return;
    }

    trap_Argv(1, stringifed_state, sizeof(stringifed_state));

    if (!DeserializeSaveState(stringifed_state, &state)) {
        trap_SendServerCommand(ent - g_entities, "print \"" LOG_ERROR
                                                 "Corrupted/invalid state\n\"");
        return;
    }

    if (ps->pm_type != PM_NORMAL) {
        trap_SendServerCommand(ent - g_entities,
                               "print \"" LOG_ERROR
                               "Must be in normal gameplay to restore state "
                               "(not spec/noclip/etc.)\n\"");
        return;
    }

    if ((state.pm_flags & PMF_PROMODE) != (ps->pm_flags & PMF_PROMODE)) {
        trap_SendServerCommand(ent - g_entities,
                               "print \"" LOG_ERROR
                               "Different physics from state\n\"");
        return;
    }

    if (
       (state.powerups[PW_REDFLAG] && ps->persistant[PERS_TEAM] != TEAM_BLUE) ||
       (state.powerups[PW_BLUEFLAG] && ps->persistant[PERS_TEAM] != TEAM_RED)
    ) {
        trap_SendServerCommand(ent - g_entities,
                               "print \"" LOG_ERROR
                               "Different team from state\n\"");
        return;
    }

    RestoreState(ent, &state);
    trap_SendServerCommand(ent - g_entities,
                           "print \"" LOG_INFO "Restored\n\"");
}
