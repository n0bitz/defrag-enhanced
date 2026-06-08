#include "cgame.h"

/*
=================
CG_Draw2D_df
=================
*/
DEFINE_HOOK(void, CG_Draw2D_df, (stereoFrame_t stereoFrame))
    CG_AddEntityPOIs();
    CG_DrawPOIs();
    ORIGINAL(CG_Draw2D_df)(stereoFrame);
END_HOOK

/*
=================
CG_DrawActive
=================
*/
DEFINE_HOOK(void, CG_DrawActive, (stereoFrame_t stereoFrame))
    CG_DrawEntityConnections();
    ORIGINAL(CG_DrawActive)(stereoFrame);
END_HOOK

/*
=================
CG_DrawBoundingBox
=================
*/
void CG_DrawBoundingBox(const vec3_t origin, const vec3_t mins,
                        const vec3_t maxs, const byte color[4])
{
    int i;
    float extx, exty, extz;
    polyVert_t verts[4];
    vec3_t corners[8];
    static qhandle_t bbox_shader = 0;
    static qhandle_t bbox_shader_nocull = 0;

    if (!bbox_shader) {
        bbox_shader = trap_R_RegisterShader("bbox");
        bbox_shader_nocull = trap_R_RegisterShader("bbox_nocull");
    }

    // get the extents (size)
    extx = maxs[0] - mins[0];
    exty = maxs[1] - mins[1];
    extz = maxs[2] - mins[2];

    // set the polygon's texture coordinates
    verts[0].st[0] = 0;
    verts[0].st[1] = 0;
    verts[1].st[0] = 0;
    verts[1].st[1] = 1;
    verts[2].st[0] = 1;
    verts[2].st[1] = 1;
    verts[3].st[0] = 1;
    verts[3].st[1] = 0;

    // set the polygon's vertex colors
    for (i = 0; i < 4; i++) {
        verts[i].modulate[0] = color[0];
        verts[i].modulate[1] = color[1];
        verts[i].modulate[2] = color[2];
        verts[i].modulate[3] = color[3];
    }

    VectorAdd(origin, maxs, corners[3]);

    VectorCopy(corners[3], corners[2]);
    corners[2][0] -= extx;

    VectorCopy(corners[2], corners[1]);
    corners[1][1] -= exty;

    VectorCopy(corners[1], corners[0]);
    corners[0][0] += extx;

    for (i = 0; i < 4; i++) {
        VectorCopy(corners[i], corners[i + 4]);
        corners[i + 4][2] -= extz;
    }

    // top
    VectorCopy(corners[0], verts[0].xyz);
    VectorCopy(corners[1], verts[1].xyz);
    VectorCopy(corners[2], verts[2].xyz);
    VectorCopy(corners[3], verts[3].xyz);
    trap_R_AddPolyToScene(bbox_shader, 4, verts);

    // bottom
    VectorCopy(corners[7], verts[0].xyz);
    VectorCopy(corners[6], verts[1].xyz);
    VectorCopy(corners[5], verts[2].xyz);
    VectorCopy(corners[4], verts[3].xyz);
    trap_R_AddPolyToScene(bbox_shader, 4, verts);

    // top side
    VectorCopy(corners[3], verts[0].xyz);
    VectorCopy(corners[2], verts[1].xyz);
    VectorCopy(corners[6], verts[2].xyz);
    VectorCopy(corners[7], verts[3].xyz);
    trap_R_AddPolyToScene(bbox_shader_nocull, 4, verts);

    // left side
    VectorCopy(corners[2], verts[0].xyz);
    VectorCopy(corners[1], verts[1].xyz);
    VectorCopy(corners[5], verts[2].xyz);
    VectorCopy(corners[6], verts[3].xyz);
    trap_R_AddPolyToScene(bbox_shader_nocull, 4, verts);

    // right side
    VectorCopy(corners[0], verts[0].xyz);
    VectorCopy(corners[3], verts[1].xyz);
    VectorCopy(corners[7], verts[2].xyz);
    VectorCopy(corners[4], verts[3].xyz);
    trap_R_AddPolyToScene(bbox_shader_nocull, 4, verts);

    // bottom side
    VectorCopy(corners[1], verts[0].xyz);
    VectorCopy(corners[0], verts[1].xyz);
    VectorCopy(corners[4], verts[2].xyz);
    VectorCopy(corners[5], verts[3].xyz);
    trap_R_AddPolyToScene(bbox_shader_nocull, 4, verts);
}
