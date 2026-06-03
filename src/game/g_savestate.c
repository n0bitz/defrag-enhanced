#include "qagame.h"

void Cmd_RestoreState_f(gentity_t* ent)
{
    int i;
    char stringifed_state[MAX_TOKEN_CHARS];
    saveState_t state;
    playerState_t* ps = &ent->client->ps;
    vec3_t origin, viewangles, velocity;

    trap_Argv(1, stringifed_state, sizeof(stringifed_state));
    // TODO: Validation
    DeserializeSaveState(stringifed_state, &state);

    ps->pm_flags = state.pm_flags;
    ps->pm_time = state.pm_time;
    VectorCopy(state.origin, origin);
    VectorCopy(state.velocity, velocity);
    ps->weaponTime = state.weaponTime;
    VectorCopy(state.viewangles, viewangles);
    ent->health = state.health;
    ps->stats[STAT_HEALTH] = state.health;
    ps->stats[STAT_HOLDABLE_ITEM] = state.holdable;
    ps->stats[STAT_WEAPONS] = state.weapons;
    ps->stats[STAT_ARMOR] = state.armor;
    ps->stats[STAT_JUMPTIME] = state.jump_time;
    ps->stats[STAT_DJING] = state.djing;
    ps->persistant[PERS_SCORE] = state.score;
    for (i = 0; i < MAX_POWERUPS; i++) {
        ps->powerups[i] = state.powerups[i];
        if (ps->powerups[i] == 0) continue;
        if (i == PW_REDFLAG || i == PW_BLUEFLAG) {
            ps->powerups[i] = INT_MAX;
        } else {
            ps->powerups[i] += level.time - state.serverTime;
        }
    }
    memcpy(ps->ammo, state.ammo, sizeof(ps->ammo));
    ps->weapon = state.weapon;
    ps->weaponstate = state.weaponstate;
    timers[ent - g_entities].time = state.timer_time;
    timers[ent - g_entities].timer_running = state.timer_running;

    DF_PlacePlayerTeleport(ent, origin, viewangles, velocity);
    trap_SendServerCommand(ent - g_entities, "print \"^3Restored\n\"");
}
