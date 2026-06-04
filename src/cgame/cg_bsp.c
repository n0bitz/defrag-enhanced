#include "cgame.h"

#define MAX_MAP_MODELS 0x400

#define LUMP_ENTITIES 0
#define LUMP_SHADERS 1
#define LUMP_PLANES 2
#define LUMP_NODES 3
#define LUMP_LEAFS 4
#define LUMP_LEAFSURFACES 5
#define LUMP_LEAFBRUSHES 6
#define LUMP_MODELS 7
#define LUMP_BRUSHES 8
#define LUMP_BRUSHSIDES 9
#define LUMP_DRAWVERTS 10
#define LUMP_DRAWINDEXES 11
#define LUMP_FOGS 12
#define LUMP_SURFACES 13
#define LUMP_LIGHTMAPS 14
#define LUMP_LIGHTGRID 15
#define LUMP_VISIBILITY 16
#define HEADER_LUMPS 17

typedef struct {
    int fileofs, filelen;
} lump_t;

typedef struct {
    int ident;
    int version;
    lump_t lumps[HEADER_LUMPS];
} dheader_t;

typedef struct {
    float mins[3], maxs[3];
    int firstSurface, numSurfaces;
    int firstBrush, numBrushes;
} dmodel_t;

static dmodel_t models[MAX_MAP_MODELS];

entity_t entities[MAX_GENTITIES];
int num_entities;

static int FindTargets(entity_t* src, entity_t** list, int max_count)
{
    int num_targets = 0;
    entity_t* dst;

    for (dst = entities; dst != entities + num_entities; dst++) {
        if (src == dst || !dst->targetname) continue;
        if (!Q_stricmp(src->target, dst->targetname)) {
            if (list && num_targets < max_count) {
                list[num_targets] = dst;
            }
            num_targets++;
        }
    }

    return num_targets;
}

static void LinkTargets(void)
{
    entity_t* src;

    for (src = entities; src != entities + num_entities; src++) {
        int num_targets;

        if (!src->target || !src->target[0]) continue;

        num_targets = FindTargets(src, NULL, 0);
        if (!num_targets) continue;

        src->targets =
           (entity_t**)CG_Alloc((num_targets + 1) * sizeof(src->targets[0]));
        if (!src->targets) continue;

        FindTargets(src, src->targets, num_targets);
        src->targets[num_targets++] = NULL;
    }
}

static void LoadModels(fileHandle_t f, lump_t* lump)
{
    if (lump->filelen < 0 || lump->filelen > sizeof(models)) {
        Com_Printf(LOG_WARN "bad model lump size\n");
        return;
    }
    trap_FS_Seek(f, lump->fileofs, FS_SEEK_SET);
    trap_FS_Read(models, lump->filelen, f);
}

static void LoadEntities(fileHandle_t f, lump_t* lump)
{
    static char entities_lump[512 * 1024];
    char* p = entities_lump;
    entity_t* ent;

    if (lump->filelen < 0 || lump->filelen > sizeof(entities_lump)) {
        Com_Printf(LOG_WARN "bad entities lump size\n");
        return;
    }
    trap_FS_Seek(f, lump->fileofs, FS_SEEK_SET);
    trap_FS_Read(entities_lump, lump->filelen, f);

    for (
       ent = entities; ent != entities + MAX_GENTITIES; ent++, num_entities++
    ) {
        char* token = COM_Parse(&p);
        if (!p) break;

        for (;;) {
            token = COM_Parse(&p);
            if (token[0] == '}') break;
            if (!Q_stricmp(token, "classname")) {
                token = COM_Parse(&p);
                ent->classname = CG_strdup(token);
            } else if (!Q_stricmp(token, "target")) {
                token = COM_Parse(&p);
                ent->target = CG_strdup(token);
            } else if (!Q_stricmp(token, "targetname")) {
                token = COM_Parse(&p);
                ent->targetname = CG_strdup(token);
            } else if (!Q_stricmp(token, "origin")) {
                float* origin = ent->origin;
                token = COM_Parse(&p);
                sscanf(token, "%f %f %f", &origin[0], &origin[1], &origin[2]);
            } else if (!Q_stricmp(token, "model")) {
                token = COM_Parse(&p);
                if (token[0] == '*') {
                    int model = atoi(token + 1);
                    VectorAdd(models[model].mins, models[model].maxs,
                              ent->origin);
                    VectorScale(ent->origin, 0.5f, ent->origin);
                }
            }
        }
    }

    LinkTargets();
}

void CG_LoadBSP(const char* filename)
{
    dheader_t header;
    fileHandle_t f;

    trap_FS_FOpenFile(filename, &f, FS_READ);
    trap_FS_Read(&header, sizeof(header), f);

    LoadModels(f, &header.lumps[LUMP_MODELS]);
    LoadEntities(f, &header.lumps[LUMP_ENTITIES]);
    trap_FS_FCloseFile(f);
}
