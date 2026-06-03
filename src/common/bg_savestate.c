#include "bg_savestate.h"
#include "bg_public.h"
#include "base64.h"

void SerializeSaveState(const saveState_t* state, char* out)
{
    // TODO: normalize to little endian before base64ing
    Base64_Encode(state, sizeof(*state), out);
    // TODO: should encode just do this?
    out[BASE64_ENCODED_LEN(sizeof(*state))] = '\0';
}

qboolean DeserializeSaveState(const char* str, saveState_t* out)
{
    int str_len = strlen(str);

    if (str_len != BASE64_ENCODED_LEN(sizeof(saveState_t))) {
        return qfalse;
    }

    Base64_Decode(str, str_len, out);
    // TODO: bring it back to host endian if needed

    if (out->holdable < HI_NONE || out->holdable >= HI_NUM_HOLDABLE) {
        return qfalse;
    }

    if (out->weapon < WP_NONE || out->weapon >= WP_NUM_WEAPONS) {
        return qfalse;
    }

    if (out->weaponstate < WEAPON_READY || out->weaponstate > WEAPON_FIRING) {
        return qfalse;
    }

    return qtrue;
}
