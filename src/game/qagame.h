#ifndef QAGAME_HEADER_GUARD__
#define QAGAME_HEADER_GUARD__

#include "assert.h"
#include "fs.h"
#include "hook.h"
#include "log.h"
#include "g_local.h"
#include "bg_savestate.h"

//
// g_cmds.c
//
// TODO: maybe consider moving this out to a separate header if it gets too big
#define FOR_EACH_CLIENT_COMMAND(V) V(RESTORE_STATE_CMD, Cmd_RestoreState_f)

#define DECLARE_COMMAND_(name, func) void func(gentity_t* ent);
FOR_EACH_CLIENT_COMMAND(DECLARE_COMMAND_)
#undef DECLARE_COMMAND_

//
// og df stuff that you don't know/care where to place
//
extern df_t df;

void DF_ResetClientWaits(int clientNum);
void DF_PlacePlayerTeleport(gentity_t* ent, vec3_t origin, vec3_t angles,
                            vec3_t velocity);
gentity_t* SelectRandomFurthestSpawnPoint(vec3_t avoidPoint, vec3_t origin,
                                          vec3_t angles, int arena);

#endif  // QAGAME_HEADER_GUARD__
