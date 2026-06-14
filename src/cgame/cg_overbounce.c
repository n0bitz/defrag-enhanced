#include "cgame.h"

#define MAX_MAP_BOUNDS 65535

static vec3_t* ClipPoly(vec3_t* poly, dplane_t* plane)
{
    vec3_t* out = NULL;
    vec3_t a, b;
    float a_dist, b_dist;
    int i;

    if (vec_len(poly) < 3) return NULL;

    VectorCopy(vec_last(poly), a);
    for (i = 0; i < vec_len(poly); i++) {
        VectorCopy(poly[i], b);

        a_dist = DotProduct(a, plane->normal) - plane->dist;
        b_dist = DotProduct(b, plane->normal) - plane->dist;

        if (a_dist <= 0.0) {
            vec3_t* o = vec_reserve(out, 1);
            VectorCopy(a, *o);
        }

        if (a_dist * b_dist < 0.0) {
            vec3_t* o = vec_reserve(out, 1);
            VectorSubtract(b, a, *o);
            VectorMA(a, a_dist / (a_dist - b_dist), *o, *o);
        }

        VectorCopy(b, a);
    }

    return out;
}

typedef struct floorSurface_s {
    vec3_t* vertices;
} floorSurface_t;

typedef struct {
    float z;
    qboolean g, j;
    float smooth_g, smooth_j;
    floorSurface_t* surfaces;
} floor_t;

static hashmap(float, floor_t) floors;

static size_t* activeFloors;

static void BuildFloors(void)
{
    int i, j, k;

    for (i = 0; i < vec_len(bsp.brushes); i++) {
        dbrush_t* brush = &bsp.brushes[i];
        floor_t floor;
        floorSurface_t* surface;

        for (j = 0; j < brush->numSides; j++) {
            dbrushside_t* side = &bsp.brushSides[brush->firstSide + j];
            dplane_t* plane = &bsp.planes[side->planeNum];

            if (
               plane->normal[2] != 1.0 ||
               !(bsp.shaders[brush->shaderNum].contentFlags &
                 MASK_PLAYERSOLID) ||
               bsp.shaders[side->shaderNum].surfaceFlags & SURF_NOOB
            ) {
                continue;
            }

            floor = hashmap_get(floors, plane->dist);
            floor.z = plane->dist;

            surface = vec_reserve(floor.surfaces, 1);
            memset(surface, 0, sizeof(*surface));

            {
                vec3_t* poly = NULL;
                vec_resize(poly, 4);
                VectorSet(poly[0], -MAX_MAP_BOUNDS, -MAX_MAP_BOUNDS, floor.z);
                VectorSet(poly[1], -MAX_MAP_BOUNDS, MAX_MAP_BOUNDS, floor.z);
                VectorSet(poly[2], MAX_MAP_BOUNDS, MAX_MAP_BOUNDS, floor.z);
                VectorSet(poly[3], MAX_MAP_BOUNDS, -MAX_MAP_BOUNDS, floor.z);

                for (k = 0; k < brush->numSides; k++) {
                    vec3_t* clippedPoly;

                    if (k == j) {
                        continue;
                    }

                    clippedPoly = ClipPoly(
                       poly, &bsp.planes[bsp.brushSides[brush->firstSide + k]
                                            .planeNum]);
                    vec_free(poly);
                    poly = clippedPoly;
                }

                surface->vertices = poly;
            }

            hashmap_insert(floors, floor.z, floor);
        }
    }
}

void CG_DrawOBs(void)
{
    static int lastFloor;
    float z, dist1, dist2;
    int i, j, k;
    float oz, vz;

    if (!cg_overbounceDraw.integer) {
        return;
    }

    if (!atoi(Info_ValueForKey(CG_ConfigString(CS_SYSTEMINFO), "defrag_obs"))) {
        return;
    }

    if (!hashmap_len(floors)) {
        CG_LoadBSP();
        BuildFloors();
        Com_Printf("^3built %d floors\n", hashmap_len(floors));
    }

    // TODO
    // [ ] respect killobs/noob (optionally? also option to show even if killobs
    //     are on but w/ a different shader?)

    oz = cg.snap->ps.origin[2];
    vz = cg.snap->ps.velocity[2];

    for (
       i = lastFloor;
       i < hashmap_len(floors) &&
       i < lastFloor + cg_overbounceMaxTestsPerFrame.integer &&
       vec_len(activeFloors) < cg_overbounceMaxTestsPerFrame.integer;
       i++
    ) {
        floor_t* floor = &floors[i].value;
        z = floor->z + 24.125;

        floor->j =
           vz == 0.0 && CheckOB(OB_JUMP, oz, vz, oz, z, &dist1, &dist2) > 0;
        floor->g = CheckOB(OB_GO, oz, vz, oz, z, &dist1, &dist2) > 0;

        if (floor->j || floor->g) {
            qboolean alreadyActive = qfalse;

            for (j = 0; j < vec_len(activeFloors); j++) {
                if (activeFloors[j] == i) {
                    alreadyActive = qtrue;
                    break;
                }
            }

            if (!alreadyActive) {
                vec_push(activeFloors, i);
            }
        }
    }

    for (i = 0; i < vec_len(activeFloors); i++) {
        int j = activeFloors[i];
        floor_t* floor = &floors[j].value;

        // If it wasn't one we just checked, check it now since it's likely not
        // an OB anymore.
        if (
           j < lastFloor ||
           j >= lastFloor + cg_overbounceMaxTestsPerFrame.integer
        ) {
            z = floor->z + 24.125;
            floor->j =
               vz == 0.0 && CheckOB(OB_JUMP, oz, vz, oz, z, &dist1, &dist2) > 0;
            floor->g = CheckOB(OB_GO, oz, vz, oz, z, &dist1, &dist2) > 0;
        }

        floor->smooth_g =
           Com_Clamp(0.0, 1.0, (floor->smooth_g * 0.9) + (floor->g * 0.1));
        floor->smooth_j =
           Com_Clamp(0.0, 1.0, (floor->smooth_j * 0.9) + (floor->j * 0.1));

        if (floor->smooth_g + floor->smooth_j < 0.001) {
            vec_remove_swap(activeFloors, i--);
        }
    }

    lastFloor += cg_overbounceMaxTestsPerFrame.integer;
    if (lastFloor > hashmap_len(floors)) {
        lastFloor = 0;
    }

    for (i = 0; i < vec_len(activeFloors); i++) {
        floor_t* floor = &floors[activeFloors[i]].value;

        for (j = 0; j < vec_len(floor->surfaces); j++) {
            floorSurface_t* surface = &floor->surfaces[j];
            polyVert_t verts[64];

            if (vec_len(surface->vertices) > 64) {
                continue;
            }

            for (k = 0; k < vec_len(surface->vertices); k++) {
                VectorCopy(surface->vertices[k], verts[k].xyz);
                verts[k].st[0] = verts[k].xyz[0] / 32.0;
                verts[k].st[1] = -verts[k].xyz[1] / 32.0;
            }

            for (k = 0; k < vec_len(surface->vertices); k++) {
                verts[k].modulate[3] = 255 * floor->smooth_j * 0.5;
            }
            trap_R_AddPolyToScene(trap_R_RegisterShader("dfe/job"),
                                  vec_len(surface->vertices), verts);

            for (k = 0; k < vec_len(surface->vertices); k++) {
                verts[k].modulate[3] = 255 * floor->smooth_g * 0.5;
            }
            trap_R_AddPolyToScene(trap_R_RegisterShader("dfe/gob"),
                                  vec_len(surface->vertices), verts);
        }
    }
}
