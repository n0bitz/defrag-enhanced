#include <keycodes.h>
#include <q_shared.h>
#include <ui_local.h>
#include "ui.h"

#define NK_IMPLEMENTATION
#define NK_ASSERT(expr)
#include "nuklear.h"

static struct nk_context ctx;
static byte memory[1024 * 1024];
static struct nk_user_font font;

static fontInfo_t idc;
vec4_t colorBlack = {0, 0, 0, 1};
static qhandle_t plzwork;

static void UIx_FillRect(float x, float y, float width, float height,
                         const float* color)
{
    trap_R_SetColor(color);

    trap_R_DrawStretchPic(x, y, width, height, 0, 0, 0, 0, plzwork);

    trap_R_SetColor(NULL);
}

static void UIx_DrawRect(float x, float y, float width, float height,
                         const float* color)
{
    trap_R_SetColor(color);

    trap_R_DrawStretchPic(x, y, width, 1, 0, 0, 0, 0, plzwork);
    trap_R_DrawStretchPic(x, y, 1, height, 0, 0, 0, 0, plzwork);
    trap_R_DrawStretchPic(x, y + height - 1, width, 1, 0, 0, 0, 0, plzwork);
    trap_R_DrawStretchPic(x + width - 1, y, 1, height, 0, 0, 0, 0, plzwork);

    trap_R_SetColor(NULL);
}

size_t strlen(const char* string)
{
    const char* s;

    s = string;
    while (*s) {
        s++;
    }
    return s - string;
}

static int Text_Width(const char* text, float scale, int limit)
{
    int count, len;
    float out;
    glyphInfo_t* glyph;
    float useScale;
    const char* s = text;
    fontInfo_t* font = &idc;
    useScale = scale * font->glyphScale;
    out = 0;
    if (text) {
        len = strlen(text);
        if (limit > 0 && len > limit) {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len) {
            if (0) {
                s += 2;
                continue;
            }
            glyph = &font->glyphs[(int)*s];
            out += glyph->xSkip;
            s++;
            count++;
        }
    }
    return out * useScale;
}

static int Text_Height(const char* text, float scale, int limit)
{
    int len, count;
    float max;
    glyphInfo_t* glyph;
    float useScale;
    const char* s = text;  // bk001206 - unsigned
    fontInfo_t* font = &idc;

    useScale = scale * font->glyphScale;
    max = 0;
    if (text) {
        len = strlen(text);
        if (limit > 0 && len > limit) {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len) {
            if (Q_IsColorString(s)) {
                s += 2;
                continue;
            }
            glyph = &font->glyphs[(
               int)*s];  // TTimo: FIXME: getting nasty warnings without the
                         // cast, hopefully this doesn't break the VM build
            if (max < glyph->height) {
                max = glyph->height;
            }
            s++;
            count++;
        }
    }
    return max * useScale;
}

static void Text_PaintChar(float x, float y, float width, float height,
                           float scale, float s, float t, float s2, float t2,
                           qhandle_t hShader)
{
    float w, h;
    w = width * scale;
    h = height * scale;
    // UI_AdjustFrom640(&x, &y, &w, &h);
    trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

static void Text_Paint(float x, float y, float scale, vec4_t color,
                       const char* text, float adjust, int limit)
{
    int len, count;
    vec4_t newColor;
    glyphInfo_t* glyph;
    float useScale;
    fontInfo_t* font = &idc;
    useScale = scale * font->glyphScale;
    if (text) {
        const char* s = text;  // bk001206 - unsigned
        trap_R_SetColor(color);
        memcpy(&newColor[0], &color[0], sizeof(vec4_t));
        len = strlen(text);
        if (limit > 0 && len > limit) {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len) {
            glyph = &font->glyphs[(
               int)*s];  // TTimo: FIXME: getting nasty warnings without the
                         // cast, hopefully this doesn't break the VM build
                         // int yadj = Assets.textFont.glyphs[text[i]].bottom +
            // Assets.textFont.glyphs[text[i]].top; float yadj = scale *
            // (Assets.textFont.glyphs[text[i]].imageHeight -
            // Assets.textFont.glyphs[text[i]].height);
            if (0) {
                // memcpy(newColor, g_color_table[ColorIndex(*(s + 1))],
                //        sizeof(newColor));
                // newColor[3] = color[3];
                // trap_R_SetColor(newColor);
                // s += 2;
                // continue;
            } else {
                float yadj = useScale * glyph->top;
                // if (style == 0) {
                //     int ofs = style == 0 ? 1 : 2;
                //     colorBlack[3] = newColor[3];
                //     trap_R_SetColor(colorBlack);
                //     Text_PaintChar(x + ofs, y - yadj + ofs,
                //     glyph->imageWidth,
                //                    glyph->imageHeight, useScale, glyph->s,
                //                    glyph->t, glyph->s2, glyph->t2,
                //                    glyph->glyph);
                //     trap_R_SetColor(newColor);
                //     colorBlack[3] = 1.0;
                // }
                Text_PaintChar(x, y - yadj, glyph->imageWidth,
                               glyph->imageHeight, useScale, glyph->s, glyph->t,
                               glyph->s2, glyph->t2, glyph->glyph);

                x += (glyph->xSkip * useScale) + adjust;
                s++;
                count++;
            }
        }
        trap_R_SetColor(NULL);
    }
}

static float text_width(nk_handle handle, float height, const char* text,
                        int len)
{
    return Text_Width(text, 16.f / 48.f, len);
    // return len * 8;
}

static void draw_poly(int num_verts, vec2_t* verts, const byte color[4])
{
    float znear = trap_Cvar_VariableValue("r_znear");
    polyVert_t poly_verts[64];
    refdef_t refdef;
    int i, j;

    if (num_verts > 64) num_verts = 64;

    for (i = 0; i < num_verts; i++) {
        for (j = 0; j < 4; j++) {
            poly_verts[i].modulate[j] = color[j];
        }
        VectorSet(poly_verts[i].xyz, verts[i][0], verts[i][1], 0);
        poly_verts[i].st[0] = poly_verts[i].st[1] = 0;
    }

    // TODO this can be caclculated once
    memset(&refdef, 0, sizeof(refdef));
    refdef.width = 1600;
    refdef.height = 900;
    refdef.fov_x = refdef.fov_y = 90;
    refdef.rdflags = RDF_NOWORLDMODEL;
    refdef.viewaxis[1][0] = -znear * 2 / refdef.width;
    refdef.viewaxis[2][1] = -znear * 2 / refdef.height;
    refdef.viewaxis[0][2] = 1;
    VectorSet(refdef.vieworg, refdef.width / 2.0, refdef.height / 2.0, -znear);

    trap_R_ClearScene();
    trap_R_AddPolyToScene(trap_R_RegisterShaderNoMip("white"), num_verts,
                          poly_verts);
    trap_R_RenderScene(&refdef);
}

DEFINE_HOOK(void, UI_Init, (void))
    ORIGINAL(UI_Init)();

    font.userdata.ptr = NULL;
    font.height = 16;
    font.width = text_width;
    nk_init_fixed(&ctx, memory, sizeof(memory), &font);

    {
        static struct nk_cursor arrow, resize;
        arrow.img.handle.id =
           trap_R_RegisterShaderNoMip("textures/cursors/arrow");
        arrow.img.w = 32;
        arrow.img.h = 32;
        arrow.size.x = 32;
        arrow.size.y = 32;
        resize.img.handle.id =
           trap_R_RegisterShaderNoMip("textures/cursors/resize");
        resize.img.w = 32;
        resize.img.h = 32;
        resize.size.x = 32;
        resize.size.y = 32;
        resize.offset.x = 8;
        resize.offset.y = 8;
        nk_style_load_cursor(&ctx, NK_CURSOR_ARROW, &arrow);
        nk_style_load_cursor(&ctx, NK_CURSOR_RESIZE_TOP_RIGHT_DOWN_LEFT,
                             &resize);
    }
    nk_style_show_cursor(&ctx);

    trap_R_RegisterFont("AdwaitaSans-Regular.ttf", 16, &idc);
    plzwork = trap_R_RegisterShaderNoMip("white");
    Com_Printf("???????????????? %d\n", Text_Height("A", 16.f / 48.f, 0));
END_HOOK

static int mouseX, mouseY, mouseDown;

DEFINE_HOOK(void, UI_KeyEvent, (int key, int down))
    if (key == K_MOUSE1) {
        mouseDown = down;
    }
    (void)ORIGINAL(UI_KeyEvent);
END_HOOK

DEFINE_HOOK(void, UI_MouseEvent, (int dx, int dy))
    mouseX += dx;
    mouseY += dy;
    (void)ORIGINAL(UI_MouseEvent);
END_HOOK

DEFINE_HOOK(void, UI_Refresh, (int realtime))
    static float value = 0.6f;

    (void)ORIGINAL(UI_Refresh);

    if (!(trap_Key_GetCatcher() & KEYCATCH_UI)) {
        return;
    }

    nk_input_begin(&ctx);
    nk_input_motion(&ctx, mouseX, mouseY);
    nk_input_button(&ctx, NK_BUTTON_LEFT, mouseX, mouseY, mouseDown);
    nk_input_end(&ctx);

    nk_begin(&ctx, "Show", nk_rect(50, 50, 220, 220),
             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE |
                NK_WINDOW_SCALABLE);
    /* fixed widget pixel width */
    nk_layout_row_static(&ctx, 30, 80, 1);
    if (nk_button_label(&ctx, "button")) {
        /* event handling */
    }

    /* custom widget pixel width */
    nk_layout_row_begin(&ctx, NK_DYNAMIC, 30, 2);
    {
        nk_layout_row_push(&ctx, 0.5);
        nk_label(&ctx, "Volume:", NK_TEXT_LEFT);
        nk_layout_row_push(&ctx, 0.5);
        nk_slider_float(&ctx, 0, &value, 1.0f, 0.1f);
    }
    nk_layout_row_end(&ctx);
    nk_end(&ctx);

    trap_Cvar_SetValue("cg_fov", 90 + (value * 60));

    nk_begin(&ctx, "Show 2", nk_rect(50, 50, 220, 220),
             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE |
                NK_WINDOW_SCALABLE);
    nk_end(&ctx);

    {
        const struct nk_command* cmd;
        nk_foreach(cmd, &ctx)
        {
            switch (cmd->type) {
                case NK_COMMAND_NOP:
                    break;
                case NK_COMMAND_LINE: {
                    const struct nk_command_line* l =
                       (const struct nk_command_line*)cmd;
                    vec2_t poly[4];
                    poly[0][0] = l->begin.x;
                    poly[0][1] = l->begin.y;
                    poly[1][0] = l->begin.x;
                    poly[1][1] = l->begin.y;
                    poly[2][0] = l->end.x;
                    poly[2][1] = l->end.y;
                    poly[3][0] = l->end.x;
                    poly[3][1] = l->end.y;
                    draw_poly(4, poly, (byte*)&l->color);
                    break;
                }
                case NK_COMMAND_RECT: {
                    const struct nk_command_rect* r =
                       (const struct nk_command_rect*)cmd;
                    struct nk_colorf color = nk_color_cf(r->color);
                    UIx_DrawRect(r->x, r->y, r->w, r->h, (float*)&color);
                    break;
                }
                case NK_COMMAND_RECT_FILLED: {
                    const struct nk_command_rect_filled* r =
                       (const struct nk_command_rect_filled*)cmd;
                    struct nk_colorf color = nk_color_cf(r->color);
                    UIx_FillRect(r->x, r->y, r->w, r->h, (float*)&color);
                    break;
                }
                case NK_COMMAND_CIRCLE: {
                    const struct nk_command_circle* c =
                       (const struct nk_command_circle*)cmd;
                    struct nk_colorf color = nk_color_cf(c->color);
                    UIx_DrawRect(c->x, c->y, c->w, c->h, (float*)&color);
                    break;
                }
                case NK_COMMAND_CIRCLE_FILLED: {
                    const struct nk_command_circle_filled* c =
                       (const struct nk_command_circle_filled*)cmd;
#define N 8
                    vec2_t poly[N];
                    int i;
                    for (i = 0; i < N; i++) {
                        poly[i][0] =
                           c->x +
                           ((1 + cos((double)i * 2 * M_PI / N)) * c->w / 2);
                        poly[i][1] =
                           c->y +
                           ((1 + sin((double)i * 2 * M_PI / N)) * c->h / 2);
                    }
                    draw_poly(N, poly, (byte*)&c->color);
                    break;
                }
                case NK_COMMAND_TRIANGLE_FILLED: {
                    const struct nk_command_triangle_filled* t =
                       (const struct nk_command_triangle_filled*)cmd;
                    vec2_t poly[3];
                    poly[0][0] = t->a.x;
                    poly[0][1] = t->a.y;
                    poly[1][0] = t->b.x;
                    poly[1][1] = t->b.y;
                    poly[2][0] = t->c.x;
                    poly[2][1] = t->c.y;
                    draw_poly(3, poly, (byte*)&t->color);
                    break;
                }
                case NK_COMMAND_POLYGON_FILLED: {
                    const struct nk_command_polygon_filled* p =
                       (const struct nk_command_polygon_filled*)cmd;
                    vec2_t poly[64];
                    int i;
                    for (i = 0; i < p->point_count && i < 64; i++) {
                        poly[i][0] = p->points[i].x;
                        poly[i][1] = p->points[i].y;
                    }
                    draw_poly(p->point_count, poly, (byte*)&p->color);
                    break;
                }
                case NK_COMMAND_TEXT: {
                    const struct nk_command_text* t =
                       (const struct nk_command_text*)cmd;
                    struct nk_colorf color = nk_color_cf(t->foreground);
                    // UI_DrawString(t->x, t->y, t->string, UI_SMALLFONT,
                    //   (float*)&color);
                    Text_Paint(t->x, t->y + t->height, 16.f / 48.f,
                               (float*)&color, t->string, 0.0f, 0);
                    break;
                }
                case NK_COMMAND_IMAGE: {
                    const struct nk_command_image* i =
                       (const struct nk_command_image*)cmd;
                    trap_R_DrawStretchPic(i->x, i->y, i->w, i->h, 0, 0, 1, 1,
                                          i->img.handle.id);
                    break;
                }
                default:
                    break;
            }
        }
    }

    nk_clear(&ctx);

    trap_R_SetColor(NULL);
END_HOOK

// DEFINE_HOOK(qboolean, UI_IsFullscreen, (void))
//     (void)ORIGINAL(UI_IsFullscreen);
//
//     return trap_Key_GetCatcher() == UIMENU_MAIN;
// END_HOOK

// DEFINE_HOOK(void, UI_SetActiveMenu, (int menu))
//     (void)ORIGINAL(UI_SetActiveMenu);
//
//     switch (menu) {
//         case UIMENU_NONE:
//             trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_UI);
//             break;
//         default:
//             trap_Key_SetCatcher(KEYCATCH_UI);
//             break;
//     }
// END_HOOK
