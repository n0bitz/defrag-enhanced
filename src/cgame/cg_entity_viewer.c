#include "cgame.h"

static vec3_t text_origins[MAX_GENTITIES];
static int spawn_points[MAX_GENTITIES], spawn_point_teams[MAX_GENTITIES],
   num_spawn_points;

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

    CG_LoadBSP();

    // bump name up if it's too close to another one to be readable
    for (i = 0; i < num_entities; i++) {
        VectorCopy(entities[i].origin, text_origins[i]);
        for (j = 0; j < i; j++) {
            if (Distance(text_origins[i], text_origins[j]) < 8.0f) {
                text_origins[i][2] += 8.0f;
            }
        }
    }

    for (i = 0; i < num_entities; i++) {
        if (
           !Q_stricmp(entities[i].classname, "info_player_deathmatch") ||
           !Q_stricmp(entities[i].classname, "info_player_start")
        ) {
            spawn_point_teams[num_spawn_points] = TEAM_FREE;
        } else if (
           !Q_stricmp(entities[i].classname, "team_CTF_redplayer") ||
           !Q_stricmp(entities[i].classname, "team_CTF_redspawn")
        ) {
            spawn_point_teams[num_spawn_points] = TEAM_RED;
        } else if (
           !Q_stricmp(entities[i].classname, "team_CTF_bluespawn") ||
           !Q_stricmp(entities[i].classname, "team_CTF_blueplayer")
        ) {
            spawn_point_teams[num_spawn_points] = TEAM_BLUE;
        } else {
            continue;
        }
        spawn_points[num_spawn_points] = i;
        num_spawn_points++;
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

void CG_DrawSpawnPoints(void)
{
    refEntity_t legs, torso, head;
    orientation_t torsoTag, headTag;
    clientInfo_t* ci;
    vec4_t freeColor, redColor, blueColor;
    int i;

    if (!cg_spawnPointsDraw.integer) {
        return;
    }

    CG_InitEntityViewer();

    ci = &cgs.clientinfo[cg.clientNum];

    memset(&legs, 0, sizeof(legs));
    legs.hModel = ci->legsModel;
    legs.frame = ci->animations[LEGS_IDLE].firstFrame;

    memset(&torso, 0, sizeof(torso));
    torso.hModel = ci->torsoModel;
    torso.frame = ci->animations[TORSO_STAND].firstFrame;
    trap_R_LerpTag(&torsoTag, legs.hModel, legs.frame, legs.frame, 0.0,
                   "tag_torso");

    memset(&head, 0, sizeof(head));
    head.hModel = ci->headModel;
    trap_R_LerpTag(&headTag, torso.hModel, torso.frame, torso.frame, 0.0,
                   "tag_head");

    legs.customShader = torso.customShader = head.customShader =
       trap_R_RegisterShader(cg_spawnPointsShader.string);

    sscanf(cg_spawnPointsColor.string, "%f %f %f %f", &freeColor[0],
           &freeColor[1], &freeColor[2], &freeColor[3]);

    if (cg_spawnPointsColorRed.string[0]) {
        sscanf(cg_spawnPointsColorRed.string, "%f %f %f %f", &redColor[0],
               &redColor[1], &redColor[2], &redColor[3]);
    } else {
        Vector4Copy(freeColor, redColor);
    }

    if (cg_spawnPointsColorBlue.string[0]) {
        sscanf(cg_spawnPointsColorBlue.string, "%f %f %f %f", &blueColor[0],
               &blueColor[1], &blueColor[2], &blueColor[3]);
    } else {
        Vector4Copy(freeColor, blueColor);
    }

    for (i = 0; i < num_spawn_points; i++) {
        entity_t* ent;
        float* color;
        float alpha;
        int j;

        ent = entities + spawn_points[i];

        switch (spawn_point_teams[i]) {
            case TEAM_RED:
                color = redColor;
                break;
            case TEAM_BLUE:
                color = blueColor;
                break;
            default:
                color = freeColor;
                break;
        }

        alpha = Com_Clamp(
           0.0, 1.0, (Distance(ent->origin, cg.snap->ps.origin) - 16.0) / 32.0);

        for (j = 0; j < 3; j++) {
            legs.shaderRGBA[j] = torso.shaderRGBA[j] = head.shaderRGBA[j] =
               color[j] * 255;
        }

        legs.shaderRGBA[3] = torso.shaderRGBA[3] = head.shaderRGBA[3] =
           color[3] * alpha * 255;

        VectorCopy(ent->origin, legs.origin);
        AnglesToAxis(ent->angles, legs.axis);

        VectorCopy(legs.origin, torso.origin);
        for (j = 0; j < 3; j++) {
            VectorMA(torso.origin, torsoTag.origin[j], legs.axis[j],
                     torso.origin);
        }
        MatrixMultiply(torsoTag.axis, legs.axis, torso.axis);

        VectorCopy(torso.origin, head.origin);
        for (j = 0; j < 3; j++) {
            VectorMA(head.origin, headTag.origin[j], torso.axis[j],
                     head.origin);
        }
        MatrixMultiply(headTag.axis, torso.axis, head.axis);

        trap_R_AddRefEntityToScene(&legs);
        trap_R_AddRefEntityToScene(&torso);
        trap_R_AddRefEntityToScene(&head);
    }
}
