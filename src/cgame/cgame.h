#ifndef CGAME_HEADER_GUARD_
#define CGAME_HEADER_GUARD_

#include "assert.h"
#include "hook.h"
#include "log.h"
#include "cg_local.h"
#include "bg_savestate.h"

// TODO: maybe consider moving this out to a separate header if it gets too big
#define FOR_EACH_CVAR(V)                  \
    V(cg_entitiesDraw, "0", CVAR_ARCHIVE) \
    V(cg_entitiesMaxDistance, "1000", CVAR_ARCHIVE)

#define DECLARE_CVAR(name, default, flags) extern vmCvar_t name;
FOR_EACH_CVAR(DECLARE_CVAR)
#undef DECLARE_CVAR

//
// cg_bsp.c
//
struct entity_s;
typedef struct entity_s entity_t;

struct entity_s {
    char* classname;
    char* target;
    char* targetname;
    entity_t** targets;
    vec3_t origin;
};

extern entity_t entities[MAX_GENTITIES];
extern int num_entities;

void CG_LoadBSP(const char* filename);

//
// cg_consolecmds.c
//
// This type alias is simply here for readability/convenience so we don't have
// to remember what true/false is or go look it up each time.
// NOTE: It is not its own enum type as we use it as CG_ConsoleCommand's
// return type and making it a separate enum would cause conflicting
// redeclaration errors with the signature declared in SDK's cg_local.h
typedef qboolean consoleCommandStatus_t;
#define CON_CMD_NOT_HANDLED qfalse
#define CON_CMD_HANDLED qtrue

#define FOR_EACH_CONSOLE_COMMAND(V) \
    V(savestate, CG_SaveState_f)    \
    V(restorestate, CG_RestoreState_f)

#define DECLARE_COMMAND(name, func) consoleCommandStatus_t func(void);
FOR_EACH_CONSOLE_COMMAND(DECLARE_COMMAND)
#undef DECLARE_COMMAND

//
// cg_entity_viewer.c
//
void CG_AddEntityPOIs(void);
void CG_AddCEntityPOI(centity_t* cent);
void CG_DrawEntityConnections(void);

//
// cg_mem.c
//
void* CG_Alloc(int size);
char* CG_strdup(const char* string);

//
// cg_poi.c
//
void CG_AddTextPOI(const vec3_t origin, const char* text, float max_dist);
void CG_DrawPOIs(void);

//
// cg_savestate.c
//
void SaveCurrentState(saveState_t* out);

//
// og df stuff that you don't know/care where to place
//
extern int timer_time;
// Updates some global timer thingy if it changed between snap and prev and
// returns the timer
int UpdateTimer(snapshot_t* snap, snapshot_t* prev);

#endif  // CGAME_HEADER_GUARD_
