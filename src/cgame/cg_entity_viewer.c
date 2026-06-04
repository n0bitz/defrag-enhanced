#include "cgame.h"

static vec3_t text_origins[MAX_GENTITIES];

static float DistanceToSegment(vec3_t point, const vec3_t a, const vec3_t b)
{
    vec3_t aToPoint, aToB, closest;
    float fraction;

    VectorSubtract(point, a, aToPoint);
    VectorSubtract(b, a, aToB);
    fraction = DotProduct(aToPoint, aToB) / DotProduct(aToB, aToB);
    VectorMA(a, Com_Clamp(0.0f, 1.0f, fraction), aToB, closest);
    return Distance(point, closest);
}

static void DrawConnection(vec3_t src, vec3_t dst)
{
    static qhandle_t shader = 0;
    refEntity_t ent;
    float dist;
    float alpha;

    dist = DistanceToSegment(cg.refdef.vieworg, src, dst);
    if (dist > cg_entitiesMaxDistance.value) return;
    alpha = 1.0f - (dist / cg_entitiesMaxDistance.value);

    if (!shader) shader = trap_R_RegisterShader("railCore");

    memset(&ent, 0, sizeof(ent));

    VectorCopy(src, ent.origin);
    VectorCopy(dst, ent.oldorigin);
    AxisClear(ent.axis);

    ent.reType = RT_RAIL_CORE;
    ent.customShader = shader;
    ent.shaderRGBA[0] = 0;
    ent.shaderRGBA[1] = alpha * 64;
    ent.shaderRGBA[2] = alpha * 128;
    ent.shaderRGBA[3] = 255;
    ent.renderfx = RF_NOSHADOW;

    trap_R_AddRefEntityToScene(&ent);
}

static void CG_InitEntityViewer(void)
{
    static qboolean initialized;
    int i, j;

    if (initialized) {
        return;
    }

    CG_LoadBSP(cgs.mapname);

    // bump name up if it's too close to another one to be readable
    for (i = 0; i < num_entities; i++) {
        VectorCopy(entities[i].origin, text_origins[i]);
        for (j = 0; j < i; j++) {
            if (Distance(text_origins[i], text_origins[j]) < 8.0f) {
                text_origins[i][2] += 8.0f;
            }
        }
    }

    initialized = qtrue;
}

void CG_AddEntityPOIs(void)
{
    int i;

    if (!cg_entitiesDraw.integer) {
        return;
    }

    CG_InitEntityViewer();

    for (i = 0; i < num_entities; i++) {
        CG_AddTextPOI(text_origins[i], entities[i].classname,
                      cg_entitiesMaxDistance.value);
    }
}

void CG_DrawEntityConnections(void)
{
    entity_t *src, **dst;

    if (!cg_entitiesDraw.integer) {
        return;
    }

    CG_InitEntityViewer();

    for (src = entities; src != entities + num_entities; src++) {
        if (!src->targets) continue;
        for (dst = src->targets; *dst; dst++) {
            DrawConnection(src->origin, (*dst)->origin);
        }
    }
}

static char* ET_NAMES[] = {
   "ET_GENERAL",      "ET_PLAYER",
   "ET_ITEM",         "ET_MISSILE",
   "ET_MOVER",        "ET_BEAM",
   "ET_PORTAL",       "ET_SPEAKER",
   "ET_PUSH_TRIGGER", "ET_TELEPORT_TRIGGER",
   "ET_INVISIBLE",    "ET_GRAPPLE",
   "ET_TEAM",
};

static char* WP_NAMES[] = {
   "", "", "", "", "grenade", "rocket", "", "", "plasma", "bfg", "hook",
};

void CG_AddCEntityPOI(centity_t* cent)
{
    entityState_t* s = &cent->currentState;
    char* text = NULL;

    if (!cg_entitiesDraw.integer) {
        return;
    }

    switch (cent->currentState.eType) {
        case ET_PLAYER:
            break;

        case ET_ITEM:
            text = bg_itemlist[s->modelindex].classname;
            break;

        case ET_MISSILE:
            text = WP_NAMES[s->weapon];
            break;

        default:
            if (s->eType >= ET_EVENTS) {
                text = va("Event #%d", s->eType - ET_EVENTS);
            } else {
                text = ET_NAMES[cent->currentState.eType];
            }
            break;
    }

    if (text) {
        CG_AddTextPOI(cent->lerpOrigin, text, cg_entitiesMaxDistance.value);
    }
}
