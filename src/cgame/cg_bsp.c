#include "cgame.h"

bsp_t bsp;

#define LoadRawLump(f, lump, dest)                          \
    (vec_resize(dest, (lump)->filelen / sizeof((dest)[0])), \
     LoadRawLump_(f, lump, (void**)&(dest), sizeof((dest)[0]), #dest))

static void LoadRawLump_(fileHandle_t f, lump_t* lump, void** dest,
                         int structSize, const char* name)
{
    if (lump->filelen < 0 || lump->filelen % structSize) {
        Com_Printf(LOG_WARN "%s: bad lump size (%d)\n", name, lump->filelen);
        return;
    }

    trap_FS_Seek(f, lump->fileofs, FS_SEEK_SET);
    trap_FS_Read(*dest, lump->filelen, f);
}

static int FindTargets(entity_t* src, entity_t** list, int max_count)
{
    int num_targets = 0;
    entity_t* dst;

    for (
       dst = bsp.entities; dst != bsp.entities + vec_len(bsp.entities); dst++
    ) {
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

    for (
       src = bsp.entities; src != bsp.entities + vec_len(bsp.entities); src++
    ) {
        int num_targets;

        if (!src->target || !src->target[0]) continue;

        num_targets = FindTargets(src, NULL, 0);
        if (!num_targets) continue;

        src->targets =
           (entity_t**)malloc((num_targets + 1) * sizeof(src->targets[0]));
        if (!src->targets) continue;

        FindTargets(src, src->targets, num_targets);
        src->targets[num_targets++] = NULL;
    }
}

static void ParseEntity(char** p, entity_t* ent)
{
    for (;;) {
        char* token = COM_Parse(p);
        if (token[0] == '}') break;
        if (!Q_stricmp(token, "classname")) {
            token = COM_Parse(p);
            ent->classname = strdup(token);
        } else if (!Q_stricmp(token, "target")) {
            token = COM_Parse(p);
            ent->target = strdup(token);
        } else if (!Q_stricmp(token, "targetname")) {
            token = COM_Parse(p);
            ent->targetname = strdup(token);
        } else if (!Q_stricmp(token, "origin")) {
            float* origin = ent->origin;
            token = COM_Parse(p);
            sscanf(token, "%f %f %f", &origin[0], &origin[1], &origin[2]);
        } else if (!Q_stricmp(token, "angle")) {
            float* angles = ent->angles;
            VectorClear(angles);
            token = COM_Parse(p);
            sscanf(token, "%f", &angles[1]);
        } else if (!Q_stricmp(token, "angles")) {
            float* angles = ent->angles;
            token = COM_Parse(p);
            sscanf(token, "%f %f %f", &angles[0], &angles[1], &angles[2]);
        } else if (!Q_stricmp(token, "model")) {
            token = COM_Parse(p);
            if (token[0] == '*') {
                int model = atoi(token + 1);
                VectorAdd(bsp.models[model].mins, bsp.models[model].maxs,
                          ent->origin);
                VectorScale(ent->origin, 0.5f, ent->origin);
            }
        }
    }
}

static void LoadEntities(fileHandle_t f, lump_t* lump)
{
    char* entities_lump;
    char* p;
    entity_t* ent;

    p = entities_lump = malloc(lump->filelen);
    trap_FS_Seek(f, lump->fileofs, FS_SEEK_SET);
    trap_FS_Read(entities_lump, lump->filelen, f);

    while (vec_len(bsp.entities) < MAX_GENTITIES) {
        COM_Parse(&p);
        if (!p) break;
        ent = vec_reserve(bsp.entities, 1);
        ParseEntity(&p, ent);
    }

    free(entities_lump);
    LinkTargets();
}

void CG_LoadBSP(void)
{
    static qboolean loaded;
    dheader_t header;
    fileHandle_t f;

    if (loaded) {
        return;
    }

    trap_FS_FOpenFile(cgs.mapname, &f, FS_READ);
    trap_FS_Read(&header, sizeof(header), f);

    LoadRawLump(f, &header.lumps[LUMP_SHADERS], bsp.shaders);
    LoadRawLump(f, &header.lumps[LUMP_MODELS], bsp.models);
    LoadRawLump(f, &header.lumps[LUMP_PLANES], bsp.planes);
    LoadRawLump(f, &header.lumps[LUMP_BRUSHSIDES], bsp.brushSides);
    LoadRawLump(f, &header.lumps[LUMP_BRUSHES], bsp.brushes);
    LoadEntities(f, &header.lumps[LUMP_ENTITIES]);

    trap_FS_FCloseFile(f);

    loaded = qtrue;
}
