#ifndef BG_SAVESTATE_HEADER_GUARD__
#define BG_SAVESTATE_HEADER_GUARD__

#include "q_shared.h"
#include "base64.h"
#include "assert.h"

#define SERIALIZED_SAVESTATE_SIZE BASE64_ENCODED_LEN(sizeof(saveState_t))
#define RESTORE_STATE_CMD "restorestate"

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
    int num_checkpoints_hit;
} saveState_t;

static_assert(SERIALIZED_SAVESTATE_SIZE < MAX_TOKEN_CHARS,
              "saveState_t too big to serialize");

void SerializeSaveState(const saveState_t* state, char* out);

// Returns false if state is invalid.
qboolean DeserializeSaveState(const char* str, saveState_t* out);

#endif  // BG_SAVESTATE_HEADER_GUARD__
