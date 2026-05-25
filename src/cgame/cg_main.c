#include "cgame.h"

#define DEFINE_CVAR(NAME, DEFAULT, FLAGS) vmCvar_t NAME;
FOR_EACH_CVAR(DEFINE_CVAR)
#undef DEFINE_CVAR

typedef struct {
    vmCvar_t* vmCvar;
    char* cvarName;
    char* defaultString;
    int cvarFlags;
} cvarTable_t;

static cvarTable_t cvarTable[] = {
#define CVAR_TABLE_ENTRY(NAME, DEFAULT, FLAGS) {&NAME, #NAME, DEFAULT, FLAGS},
    FOR_EACH_CVAR(CVAR_TABLE_ENTRY)
#undef CVAR_TABLE_ENTRY
};

static int cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);

/*
=================
CG_RegisterCvars
=================
*/
DEFINE_HOOK(void CG_RegisterCvars, (void))
{
    int i;
    cvarTable_t* cv;

    ORIGINAL(CG_RegisterCvars)();

    for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
        trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString,
                           cv->cvarFlags);
    }
}

/*
=================
CG_UpdateCvars
=================
*/
DEFINE_HOOK(void CG_UpdateCvars, (void))
{
    int i;
    cvarTable_t* cv;

    for (i = 0, cv = cvarTable; i < cvarTableSize; i++, cv++) {
        trap_Cvar_Update(cv->vmCvar);
    }
}

/*
=================
CG_Init
=================
*/
DEFINE_HOOK(void CG_Init,
            (int serverMessageNum, int serverCommandSequence, int clientNum))
{
    int i;
    trap_Print("^0HELLO ^1WORLD ^7(CGAME)\n");
    ORIGINAL(CG_Init)(serverMessageNum, serverCommandSequence, clientNum);
}
