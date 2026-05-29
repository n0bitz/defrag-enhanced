#include "bg_savestate.h"
#include "base64.h"

void SerializeSaveState(const saveState_t* state, char* out)
{
    // TODO: normalize to little endian before base64ing
    Base64_Encode(state, sizeof(*state), out);
    // TODO: should encode just do this?
    out[BASE64_ENCODED_LEN(sizeof(*state))] = '\0';
}

void DeserializeSaveState(const char* buf, saveState_t* out)
{
    Base64_Decode(buf, strlen(buf), out);
}
