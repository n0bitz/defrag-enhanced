#ifndef CGAME_HEADER_GUARD__
#define CGAME_HEADER_GUARD__

#include "assert.h"
#include "cvar.h"
#include "hook.h"
#include "log.h"
#include "cg_local.h"
#include "bg_savestate.h"

// TODO: maybe consider moving this out to a separate header if it gets too big
#define FOR_EACH_CVAR(V)                  \
    V(cg_entitiesDraw, "0", CVAR_ARCHIVE) \
    V(cg_entitiesMaxDistance, "1000", CVAR_ARCHIVE)

#define DECLARE_CVAR_(name, default, flags) extern vmCvar_t name;
FOR_EACH_CVAR(DECLARE_CVAR_)
#undef DECLARE_CVAR_

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
    V("savestate", CG_SaveState_f)  \
    V(RESTORE_STATE_CMD, CG_RestoreState_f)

#define DECLARE_COMMAND_(name, func) consoleCommandStatus_t func(void);
FOR_EACH_CONSOLE_COMMAND(DECLARE_COMMAND_)
#undef DECLARE_COMMAND_

//
// cg_draw.c
//
void CG_DrawBoundingBox(const vec3_t origin, const vec3_t mins,
                        const vec3_t maxs, const byte color[4]);

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
// Returns false if current state is not supported.
qboolean SaveCurrentState(saveState_t* out);

//
// og df stuff that you don't know/care where to place
//
extern vmCvar_t cg_drawBBox;
extern vmCvar_t df_cl_alwaysDrawItems;
extern int is_multiplayer;
extern int sv_cheats;
extern int timer_time;

qboolean IsItemEntityAvailableToClient(entityState_t* state, int clientNum);

// Updates some global timer thingy if it changed between snap and prev and
// returns the timer
int UpdateTimer(snapshot_t* snap, snapshot_t* prev);

#endif  // CGAME_HEADER_GUARD__
