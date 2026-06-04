#include "cgame.h"

typedef struct {
    const char* name;
    consoleCommandStatus_t (*function)(void);
} consoleCommand_t;  // if the og q3 type is ever needed, prefix that with q3 :D

static consoleCommand_t commandTable[] = {
#define COMMAND_TABLE_ENTRY(name, func) {name, func},
    FOR_EACH_CONSOLE_COMMAND(COMMAND_TABLE_ENTRY)
#undef COMMAND_TABLE_ENTRY
};

static int commandTableSize = sizeof(commandTable) / sizeof(commandTable[0]);

DEFINE_HOOK(consoleCommandStatus_t, CG_ConsoleCommand, (void))
    int i;
    char cmd[MAX_TOKEN_CHARS];

    if (ORIGINAL(CG_ConsoleCommand)() == CON_CMD_HANDLED) {
        return CON_CMD_HANDLED;
    }

    trap_Argv(0, cmd, sizeof(cmd));
    for (i = 0; i < commandTableSize; i++) {
        if (!Q_stricmp(cmd, commandTable[i].name)) {
            return commandTable[i].function();
        }
    }

    return CON_CMD_NOT_HANDLED;
END_HOOK

DEFINE_HOOK(void, CG_InitConsoleCommands, (void))
    int i;

    ORIGINAL(CG_InitConsoleCommands)();

    for (i = 0; i < commandTableSize; i++) {
        trap_AddCommand(commandTable[i].name);
    }
END_HOOK
