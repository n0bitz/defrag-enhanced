#include "qagame.h"

typedef struct {
    const char* name;
    void (*function)(gentity_t* ent);
} clientCommand_t;

static clientCommand_t commandTable[] = {
#define COMMAND_TABLE_ENTRY(name, func) {#name, func},
    FOR_EACH_CLIENT_COMMAND(COMMAND_TABLE_ENTRY)
#undef COMMAND_TABLE_ENTRY
};

// For conveience and stability, this repeats ClientCommand's prologue and
// then checks for our functions before forwarding it to DF, as this is a less
// likely thing to ever change. This is unlike console commands in cgame, where
// we check for our stuff at the end. This is because the alternative would be
// to hook something at the end in DF's ClientCommand that is also able to
// alter its way out of the if-else control flow to skip the unrecognized
// command at the end. Coincidentally, there is such a thing (DF_AuthCommands),
// however there's no guarantee it stays that way. OK, maybe there is, as it's
// unlikely that DF will ever have another update. Just in case though, to make
// our lives easier to update to such a thing if we wanted, we refrain from this
// alternative. Moreover, no one should be relying on when our commands are run.
// If they want to control some existing DF command to do something before/after
// it, they should be hooking its handler to have that precise control.
DEFINE_HOOK(void, ClientCommand, (int clientNum))
    int i;
    gentity_t* ent;
    char cmd[MAX_TOKEN_CHARS];

    // BEGIN stuff that vanilla/DF also does
    ent = g_entities + clientNum;
    if (!ent->client) {
        return;  // not fully in game yet
    }

    // DF has its commands after this, so we repeat this too,
    // but does it matter? probably not
    if (level.intermissiontime) {
        return;
    }

    trap_Argv(0, cmd, sizeof(cmd));
    // END stuff that vanilla/DF also does

    for (i = 0; i < sizeof(commandTable) / sizeof(commandTable[0]); i++) {
        if (!Q_stricmp(cmd, commandTable[i].name)) {
            commandTable[i].function(ent);
            return;
        }
    }

    ORIGINAL(ClientCommand)(clientNum);
END_HOOK
