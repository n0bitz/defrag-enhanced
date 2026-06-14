#include "extensions.h"

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

        trap_GetValue = (trap_GetValue_t*)~atoi(buffer);
    }

    return trap_GetValue(value, valueSize, key);
}

qboolean GetExtension(const char* name, extensionTrap_t* trap)
{
    char value[16];

    if (trap_GetValue(value, sizeof(value), name)) {
        *trap = (extensionTrap_t)~atoi(value);
        return qtrue;
    }

    return qfalse;
}

void trap_Cvar_SetDescription_Q3E(const char* name, const char* description)
{
    static void (*trap_Cvar_SetDescription_Q3E)(const char* name,
                                                const char* description);

    if (!trap_Cvar_SetDescription_Q3E) {
        GetExtension("trap_Cvar_SetDescription_Q3E",
                     (extensionTrap_t*)&trap_Cvar_SetDescription_Q3E);
    }

    if (trap_Cvar_SetDescription_Q3E) {
        trap_Cvar_SetDescription_Q3E(name, description);
    }
}
