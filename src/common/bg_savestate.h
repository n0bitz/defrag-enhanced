#ifndef BG_SAVESTATE_HEADER_GUARD_
#define BG_SAVESTATE_HEADER_GUARD_

#include "q_shared.h"

typedef struct {
    // BEGIN ps stuff
    int pm_flags;
    int pm_time;
    vec3_t origin;
    vec3_t velocity;
    int weaponTime;
    vec3_t viewangles;
    int health;
    int holdable;
    int weapons;
    int armor;
    int jump_time;
    qboolean djing;
    int score;
    int powerups[MAX_POWERUPS];
    int ammo[MAX_WEAPONS];
    int weapon;
    int weaponstate;
    // END ps stuff
    int serverTime;
    int timer_time;
    qboolean timer_running;
} saveState_t;

void SerializeSaveState(const saveState_t* state, char* out);
// Returns false if state is invalid.
qboolean DeserializeSaveState(const char* str, saveState_t* out);

// TODO: WHAT AM I TRYING TO DO HERE?
#define RESTORE_COMMAND "restorestate"

// TODO: linter assert sizeof(RESTORE_COMMAND) + sizeof(" ") +
// sizeof(saveState_t) * b64 cap factor < MAX_TOKEN_CHARS

#endif  // BG_SAVESTATE_HEADER_GUARD_
