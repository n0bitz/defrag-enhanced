#ifndef BG_SAVESTATE_HEADER_GUARD_
#define BG_SAVESTATE_HEADER_GUARD_

#include "q_shared.h"

typedef struct {
    vec3_t origin;
    vec3_t viewangles;
    vec3_t velocity;
    int timer_time;
    qboolean timer_running;
    int weapon;
    int weaponstate;
    int weaponTime;
    int weapons;
    int ammo[MAX_WEAPONS];
    int powerups[MAX_POWERUPS];
    int health;
    int armor;
    int frags;
    int pm_flags;
    int pm_time;
    int dj_time;     // TODO: Rename?
    qboolean djing;  // TODO: Rename?
} saveState_t;

void SerializeSaveState(const saveState_t* state, char* out);
void DeserializeSaveState(const char* buf, saveState_t* out);

// TODO: WHAT AM I TRYING TO DO HERE?
#define RESTORE_COMMAND "restorestate"

// TODO: linter assert sizeof(RESTORE_COMMAND) + sizeof(" ") +
// sizeof(saveState_t) * b64 cap factor < MAX_TOKEN_CHARS

#endif  // BG_SAVESTATE_HEADER_GUARD_
