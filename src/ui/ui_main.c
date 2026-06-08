#include <q_shared.h>
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
            } else {
                glyph = &font->glyphs[(int)*s];
                out += glyph->xSkip;
                s++;
                count++;
            }
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
            } else {
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
                       const char* text, float adjust, int limit, int style)
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
    return Text_Width(text, 12.f / 48.f, len);
    // return len * 8;
}

DEFINE_HOOK(void, UI_Init, (void))
    ORIGINAL(UI_Init)();

    font.userdata.ptr = NULL;
    font.height = 16;
    font.width = text_width;
    nk_init_fixed(&ctx, memory, sizeof(memory), &font);
    trap_R_RegisterFont("AdwaitaSans-Regular.ttf", 16, &idc);
    plzwork = trap_R_RegisterShaderNoMip("white");
    Com_Printf("???????????????? %d\n", Text_Height("A", 12.f / 48.f, 0));
END_HOOK

// DEFINE_HOOK(void, UI_KeyEvent, (int key, int down))
//     (void)ORIGINAL(UI_KeyEvent);
// END_HOOK

// DEFINE_HOOK(void, UI_MouseEvent, (int dx, int dy))
//     (void)ORIGINAL(UI_MouseEvent);
// END_HOOK

DEFINE_HOOK(void, UI_Refresh, (int realtime))
    static float value = 0.6f;

    (void)ORIGINAL(UI_Refresh);

    if (!(trap_Key_GetCatcher() & KEYCATCH_UI)) {
        return;
    }

    nk_begin(&ctx, "Show", nk_rect(50, 50, 220, 220),
             NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE);
    /* fixed widget pixel width */
    nk_layout_row_static(&ctx, 30, 80, 1);
    if (nk_button_label(&ctx, "button")) {
        /* event handling */
    }

    /* custom widget pixel width */
    nk_layout_row_begin(&ctx, NK_STATIC, 30, 2);
    {
        nk_layout_row_push(&ctx, 50);
        nk_label(&ctx, "Volume:", NK_TEXT_LEFT);
        nk_layout_row_push(&ctx, 110);
        nk_slider_float(&ctx, 0, &value, 1.0f, 0.1f);
    }
    nk_layout_row_end(&ctx);
    nk_end(&ctx);

    {
        const struct nk_command* cmd;
        nk_foreach(cmd, &ctx)
        {
            switch (cmd->type) {
                case NK_COMMAND_NOP:
                    break;
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
                case NK_COMMAND_TEXT: {
                    const struct nk_command_text* t =
                       (const struct nk_command_text*)cmd;
                    struct nk_colorf color = nk_color_cf(t->foreground);
                    // UI_DrawString(t->x, t->y, t->string, UI_SMALLFONT,
                    //   (float*)&color);
                    Text_Paint(t->x, t->y + t->height, 12.f / 48.f,
                               (float*)&color, t->string, 0.0f, 0, 0);
                    break;
                }
                default:
                    break;
            }
        }
    }

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
