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

    VectorCopy(state.origin, origin);
    VectorCopy(state.viewangles, viewangles);
    VectorCopy(state.velocity, velocity);
    timers[ent - g_entities].time = state.timer_time;
    timers[ent - g_entities].timer_running = state.timer_running;
    ps->weapon = state.weapon;
    for (i = 0; i < MAX_WEAPONS; i++) {
        ps->ammo[i] = state.ammo[i];
    }
    for (i = 0; i < MAX_POWERUPS; i++) {
        ps->powerups[i] = state.powerups[i];
        if (ps->powerups[i] == -1) {
            ps->powerups[i] = 0;
        } else if (i == PW_REDFLAG || i == PW_BLUEFLAG) {
            ps->powerups[i] = INT_MAX;
        } else {
            ps->powerups[i] += level.time;
        }
    }
    ent->health = state.health;
    ps->stats[STAT_HEALTH] = state.health;
    ps->stats[STAT_ARMOR] = state.armor;
    ps->pm_flags = state.pm_flags;
    ps->pm_time = state.pm_time;
    ps->weaponstate = state.weaponstate;
    ps->weaponTime = state.weaponTime;
    ps->stats[STAT_WEAPONS] = state.weapons;
    ps->persistant[PERS_SCORE] = state.frags;
    ps->stats[STAT_JUMPTIME] = state.dj_time;
    ps->stats[STAT_DJING] = state.djing;

    DF_PlacePlayerTeleport(ent, origin, viewangles, velocity);
    trap_SendServerCommand(ent - g_entities, "print \"^3Restored\n\"");
}
