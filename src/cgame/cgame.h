#ifndef CGAME_HEADER_GUARD_
#define CGAME_HEADER_GUARD_

#include "assert.h"
#include "hook.h"
#include "log.h"
#include "cg_local.h"

#define FOR_EACH_CVAR(V)                  \
    V(cg_entitiesDraw, "0", CVAR_ARCHIVE) \
    V(cg_entitiesMaxDistance, "1000", CVAR_ARCHIVE)

#define DECLARE_CVAR(NAME, DEFAULT, FLAGS) extern vmCvar_t NAME;
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

#endif  // CGAME_HEADER_GUARD_
