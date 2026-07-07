#include "bg_savestate.h"
#include "bg_public.h"

void SerializeSaveState(const saveState_t* state, char* out)
{
    // TODO: normalize to little endian before base64ing
    Base64_Encode(state, sizeof(*state), out);
}

qboolean DeserializeSaveState(const char* str, saveState_t* out)
{
    int str_len = strlen(str);

    if (str_len != SERIALIZED_SAVESTATE_SIZE) {
        return qfalse;
    }

    Base64_Decode(str, str_len, out);
    // TODO: bring it back to host endian (if host is different)

    // The following checks are to make sure things to don't go OOB and such,
    // not so much as to prevent/disallow bogus/broken states.
    if (out->holdable < 0 || out->holdable >= bg_numItems) {
        return qfalse;
    }
    if (out->holdable > 0 && bg_itemlist[out->holdable].giType != IT_HOLDABLE) {
        return qfalse;
    }

    if (out->weapon < WP_NONE || out->weapon >= WP_NUM_WEAPONS) {
        return qfalse;
    }
    if (out->weaponstate < WEAPON_READY || out->weaponstate > WEAPON_FIRING) {
        return qfalse;
    }

    if (out->num_checkpoints_hit < 0 || out->num_checkpoints_hit >= 0x20) {
        return qfalse;
    }

    return qtrue;
}
