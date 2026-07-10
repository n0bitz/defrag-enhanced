#include "cgame.h"

#define MAX_MAP_BOUNDS 65535
#define MAX_POLY_VERTS 64

typedef struct floorSurface_s {
    vec(vec3_t) vertices;
    qboolean noOB;
} floorSurface_t;

typedef struct {
    float z;
    qboolean g, j;
    float smooth_g, smooth_j;
    vec(floorSurface_t) surfaces;
} floor_t;

static hashmap(float, floor_t) floors;
static vec(size_t) activeFloors;

// This is a reimplementation of DF's function but with killOBs/NOOB checks.
DEFINE_HOOK(void, UpdateOB, (vec3_t origin, vec3_t velocity, vec3_t viewangles))
    obType_t obType;
    qboolean traced = qfalse;
    float normalZ;
    trace_t downTrace, crosshairTrace;
    qboolean killOBs;

    (void)ORIGINAL(UpdateOB);

    if (!cg.snap) {
        return;
    }

    // DFE change: respect df_ob_KillOBs
    killOBs = !cg_overbounceIgnoreKillOBs.integer && !defragInfo.obs;

    for (obType = 0; obType < OB_MAX; obType++) {
        obStatus[obType] = -1;
        obOffset[obType] = 0;
        obLastOffset[obType] = 0;

        // DFE change: respect df_ob_KillOBs
        if (killOBs || !NeedOBInfo(obType)) {
            continue;
        }

        if (!traced) {
            TraceDown(&downTrace, origin);
            TraceCrosshair(&crosshairTrace, origin, viewangles);

            // DFE change: respect SURF_NOOB
            if (!cg_overbounceIgnoreNoOB.integer) {
                if (downTrace.surfaceFlags & SURF_NOOB) {
                    downTrace.plane.normal[2] = -1;
                }
                if (crosshairTrace.surfaceFlags & SURF_NOOB) {
                    crosshairTrace.plane.normal[2] = -1;
                }
            }

            traced = qtrue;
        }

        normalZ = obType == OB_BELOW ? downTrace.plane.normal[2]
                                     : crosshairTrace.plane.normal[2];

        if (normalZ > 0 && (normalZ == 1 || df_ob_AllSlopes.integer)) {
            obStatus[obType] =
               CheckOB(obType, origin[2], velocity[2], downTrace.endpos[2],
                       crosshairTrace.endpos[2], &obOffset[obType],
                       &obLastOffset[obType]);
        }
    }

    obCurrentWeapon = (obCurrentWeapon + 1) % 4;
    obCurrentStickyType = (obCurrentStickyType + 1) % 3;
END_HOOK

static vec(vec3_t) BuildPolyFromBrushSide(dbrush_t* brush, dbrushside_t* side)
{
    int i;
    dplane_t* plane = &bsp.planes[side->planeNum];

    vec(vec3_t) poly = NULL;
    vec_resize(poly, 4);
    VectorSet(poly[0], -MAX_MAP_BOUNDS, -MAX_MAP_BOUNDS, plane->dist);
    VectorSet(poly[1], -MAX_MAP_BOUNDS, MAX_MAP_BOUNDS, plane->dist);
    VectorSet(poly[2], MAX_MAP_BOUNDS, MAX_MAP_BOUNDS, plane->dist);
    VectorSet(poly[3], MAX_MAP_BOUNDS, -MAX_MAP_BOUNDS, plane->dist);

    for (i = 0; i < brush->numSides; i++) {
        dbrushside_t* clipSide = &bsp.brushSides[brush->firstSide + i];
        vec(vec3_t) clippedPoly;

        if (clipSide == side) {
            continue;
        }

        clippedPoly = ClipPoly(poly, &bsp.planes[clipSide->planeNum]);
        vec_free(poly);
        poly = clippedPoly;
    }

    if (vec_len(poly) < 3) {
        vec_free(poly);
        return NULL;
    }

    return poly;
}

static void BuildFloors(void)
{
    vec3_t* poly;
    int i, j;

    // just worldspawn for now
    int firstBrush = bsp.models[0].firstBrush;
    int numBrushes = bsp.models[0].numBrushes;

    for (i = firstBrush; i < firstBrush + numBrushes; i++) {
        dbrush_t* brush = &bsp.brushes[i];
        dshader_t* shader = &bsp.shaders[brush->shaderNum];
        floor_t floor;
        floorSurface_t* surface;

        if (!(shader->contentFlags & MASK_PLAYERSOLID)) {
            continue;
        }

        for (j = 0; j < brush->numSides; j++) {
            dbrushside_t* side = &bsp.brushSides[brush->firstSide + j];
            dshader_t* shader = &bsp.shaders[side->shaderNum];
            dplane_t* plane = &bsp.planes[side->planeNum];

            if (plane->normal[2] != 1.0) {
                continue;
            }

            poly = BuildPolyFromBrushSide(brush, side);

            if (poly) {
                floor = hashmap_get(floors, plane->dist);
                floor.z = plane->dist;

                surface = vec_reserve(floor.surfaces, 1);
                memset(surface, 0, sizeof(*surface));
                surface->vertices = poly;
                surface->noOB = shader->surfaceFlags & SURF_NOOB;

                hashmap_insert(floors, floor.z, floor);
                break;
            }
        }
    }
}

static void UpdateFloorOBStatus(floor_t* floor)
{
    float z, oz, vz, dist1, dist2;

    oz = cg.snap->ps.origin[2];
    vz = cg.snap->ps.velocity[2];
    z = floor->z + 0.125;

    if (z > oz + DEFAULT_VIEWHEIGHT) {
        floor->g = floor->j = qfalse;
        return;
    }

    z -= MINS_Z;
    floor->j = vz == 0.0 && CheckOB(OB_JUMP, oz, vz, oz, z, &dist1, &dist2) > 0;
    floor->g = CheckOB(OB_GO, oz, vz, oz, z, &dist1, &dist2) > 0;
}

static void FindAndActivateFloors(void)
{
    static int currentFloor;
    floor_t* floor;
    qboolean alreadyActive;
    int i, j;

    for (
       i = 0; i < cg_overbounceMaxTestsPerFrame.integer; i++, currentFloor++
    ) {
        if (currentFloor >= hashmap_len(floors)) {
            currentFloor = 0;
        }
        floor = &floors[currentFloor].value;

        UpdateFloorOBStatus(floor);
        if (!floor->j && !floor->g) {
            continue;
        }

        alreadyActive = qfalse;

        // TODO: do we want to use a hashset or something?
        for (j = 0; j < vec_len(activeFloors); j++) {
            if (activeFloors[j] == currentFloor) {
                alreadyActive = qtrue;
                break;
            }
        }

        if (!alreadyActive) {
            vec_push(activeFloors, currentFloor);
        }
    }
}

static void UpdateActiveFloors(void)
{
    float fadeStep;
    int i;

    fadeStep = cg.frametime * 0.001 /
               Com_Clamp(0.001, 10.0, cg_overbounceFadeTime.value);

    for (i = 0; i < vec_len(activeFloors); i++) {
        floor_t* floor = &floors[activeFloors[i]].value;

        floor->smooth_g = MoveToward(floor->smooth_g, floor->g, fadeStep);
        floor->smooth_j = MoveToward(floor->smooth_j, floor->j, fadeStep);

        if (
           !floor->g && !floor->j && floor->smooth_g + floor->smooth_j < 0.01
        ) {
            floor->smooth_g = floor->smooth_j = 0.0;
            vec_remove_swap(activeFloors, i--);
        }
    }
}

static void DrawPoly(polyVert_t* vertices, int numVertices, qhandle_t shader,
                     const vec4_t color)
{
    int i, c;
    for (i = 0; i < numVertices; i++) {
        for (c = 0; c < 4; c++) {
            vertices[i].modulate[c] = 255 * color[c];
        }
    }
    trap_R_AddPolyToScene(shader, numVertices, vertices);
}

static void DrawFloorSurface(floor_t* floor, floorSurface_t* surface,
                             const vec4_t hlColor)
{
    static qhandle_t hlShader, gShader, jShader;
    static qboolean registeredShaders;
    polyVert_t verts[MAX_POLY_VERTS];
    vec4_t color;
    int i;

    if (!registeredShaders) {
        hlShader = trap_R_RegisterShader("dfe/obHighlight");
        gShader = trap_R_RegisterShader("dfe/obGo");
        jShader = trap_R_RegisterShader("dfe/obJump");
        registeredShaders = qtrue;
    }

    if (vec_len(surface->vertices) > MAX_POLY_VERTS) {
        return;
    }

    for (i = 0; i < vec_len(surface->vertices); i++) {
        VectorCopy(surface->vertices[i], verts[i].xyz);
        verts[i].st[0] = verts[i].xyz[0] / 32.0;
        verts[i].st[1] = -verts[i].xyz[1] / 32.0;
    }

    Vector4Copy(hlColor, color);
    DrawPoly(verts, vec_len(surface->vertices), hlShader, color);

    if (floor->smooth_g) {
        color[3] = hlColor[3] * floor->smooth_g;
        DrawPoly(verts, vec_len(surface->vertices), gShader, color);
    }

    if (floor->smooth_j) {
        color[3] = hlColor[3] * floor->smooth_j;
        DrawPoly(verts, vec_len(surface->vertices), jShader, color);
    }
}

static void DrawActiveFloors(void)
{
    int i, j;
    vec4_t gColor, jColor;

    ParseRGBAf(cg_overbounceColorGo.string, gColor);
    ParseRGBAf(cg_overbounceColorJump.string, jColor);

    for (i = 0; i < vec_len(activeFloors); i++) {
        floor_t* floor = &floors[activeFloors[i]].value;
        vec4_t hlColor;

        Vector4Scale(gColor, floor->smooth_g, hlColor);
        Vector4MA(hlColor, floor->smooth_j, jColor, hlColor);
        Vector4Scale(hlColor,
                     1.0 / fmaxf(floor->smooth_g + floor->smooth_j, 1.0),
                     hlColor);

        for (j = 0; j < vec_len(floor->surfaces); j++) {
            floorSurface_t* surface = &floor->surfaces[j];

            if (surface->noOB && !cg_overbounceIgnoreNoOB.integer) {
                continue;
            }

            DrawFloorSurface(floor, surface, hlColor);
        }
    }
}

void CG_DrawOBs(void)
{
    static qboolean builtFloors;

    if (!cg_overbounceDraw.integer) {
        return;
    }

    if (!cg_overbounceIgnoreKillOBs.integer && !defragInfo.obs) {
        return;
    }

    if (!builtFloors) {
        CG_LoadBSP();
        BuildFloors();
        builtFloors = qtrue;
    }

    FindAndActivateFloors();
    UpdateActiveFloors();
    DrawActiveFloors();
}
