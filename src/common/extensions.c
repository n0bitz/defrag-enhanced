#include "extensions.h"
#include "assert.h"

// This is available in all VMs
void trap_Cvar_VariableStringBuffer(const char* var_name, char* buffer,
                                    int bufsize);

typedef qboolean trap_GetValue_t(char*, int, const char*);

qboolean trap_GetValue(char* value, int valueSize, const char* key)
{
    static trap_GetValue_t* trap_GetValue;
    static qboolean failed;

    if (failed) {
        return qfalse;
    }

    if (!trap_GetValue) {
        char buffer[MAX_CVAR_VALUE_STRING];
        trap_Cvar_VariableStringBuffer("//trap_GetValue", buffer,
                                       sizeof(buffer));
        if (!buffer[0]) {
            failed = qtrue;
            return qfalse;
        }

        // NOLINTNEXTLINE(performance-no-int-to-ptr)
        trap_GetValue = (trap_GetValue_t*)~atoi(buffer);
    }

    return trap_GetValue(value, valueSize, key);
}

qboolean GetTrapExtension(const char* name, trapExtension_t* trap)
{
    // Quake3e sends value as %i (eg.
    // https://github.com/ec-/Quake3e/blob/39e5984dc791bdce64b129887c0a4f23bcf5bbe6/code/client/cl_cgame.c#L420)
    char value[16];
    static_assert(sizeof(value) >= sizeof("-2147483648"),
                  "value buffer too small");

    if (trap_GetValue(value, sizeof(value), name)) {
        // NOLINTNEXTLINE(performance-no-int-to-ptr)
        *trap = (trapExtension_t)~atoi(value);
        return qtrue;
    }

    return qfalse;
}

void trap_Cvar_SetDescription_Q3E(const char* name, const char* description)
{
    static void (*trap_Cvar_SetDescription_Q3E)(const char* name,
                                                const char* description);
    static qboolean failed;

    if (failed) {
        return;
    }

    if (!trap_Cvar_SetDescription_Q3E) {
        failed =
           !GetTrapExtension("trap_Cvar_SetDescription_Q3E",
                             (trapExtension_t*)&trap_Cvar_SetDescription_Q3E);
        if (failed) {
            return;
        }
    }

    trap_Cvar_SetDescription_Q3E(name, description);
}
