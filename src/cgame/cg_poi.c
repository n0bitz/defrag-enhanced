#include "cgame.h"

static qhandle_t charsetShader;

static void DrawString(float x, float y, float width, float height, char* s,
                       float* color)
{
    x *= cgs.screenXScale;
    y *= cgs.screenYScale;
    width *= cgs.screenXScale;
    height *= cgs.screenYScale;

    trap_R_SetColor(color);
    while (*s) {
        int ch = *(unsigned char*)s++;
        float frow = (ch >> 4) * 0.0625;
        float fcol = (ch & 15) * 0.0625;
        float size = 0.0625;
        trap_R_DrawStretchPic(x, y, width, height, fcol, frow, fcol + size,
                              frow + size, charsetShader);
        x += width;
    }
    trap_R_SetColor(NULL);
}

#define MAX_POIS 1024
#define MAX_POI_CHARS 64

struct {
    vec3_t origin;
    char text[MAX_POI_CHARS];
    float max_dist;
} text_poi_list[MAX_POIS];

static int num_text_pois;

void CG_AddTextPOI(const vec3_t origin, const char* text, float max_dist)
{
    if (num_text_pois < MAX_POIS) {
        VectorCopy(origin, text_poi_list[num_text_pois].origin);
        Q_strncpyz(text_poi_list[num_text_pois].text, text, MAX_POI_CHARS);
        text_poi_list[num_text_pois].max_dist = max_dist;
        num_text_pois++;
    }
}

void CG_ClearPOIs(void) { num_text_pois = 0; }

// should be called after rendering the scene and before drawing the hud
void CG_DrawPOIs(void)
{
    static qboolean initialized;
    float alpha;
    float dist, max_dist;
    int i;

    if (!initialized) {
        // TODO fix cgs offsets and use cgs.media.charsetShader or whatever
        charsetShader = trap_R_RegisterShader("gfx/2d/bigchars");
    }

    for (i = 0; i < num_text_pois; i++) {
        dist = Distance(text_poi_list[i].origin, cg.refdef.vieworg);
        max_dist = text_poi_list[i].max_dist;

        if (dist > max_dist) {
            continue;
        }

        alpha = 1.0f - (dist / max_dist);

        if (text_poi_list[i].text[0]) {
            vec4_t color = {1.0f, 1.0f, 1.0f, 1.0f};
            vec3_t o, t;
            float x, y, dx, dy, w, h;
            int len;

            VectorSubtract(text_poi_list[i].origin, cg.refdef.vieworg, o);

            if (DotProduct(o, cg.refdef.viewaxis[0]) >= 0) {
                VectorRotate(o, cg.refdef.viewaxis, t);

                // distances from viewer to screen in pixels
                dx = 320.0f / tan(DEG2RAD(cg.refdef.fov_x * 0.5));
                dy = 240.0f / tan(DEG2RAD(cg.refdef.fov_y * 0.5));

                x = -dx * t[1] / t[0];
                y = -dy * t[2] / t[0];
                w = dx * 6 / t[0];
                h = dy * 8 / t[0];

                color[3] = alpha;
                len = CG_DrawStrlen(text_poi_list[i].text);

                DrawString(320 + x - (len * w / 2), 240 + y - (h / 2), w, h,
                           text_poi_list[i].text, color);
            }
        }
    }

    CG_ClearPOIs();
}
