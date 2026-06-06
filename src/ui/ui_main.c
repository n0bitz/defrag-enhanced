#include <q_shared.h>
#include "ui.h"

#define NK_IMPLEMENTATION
#define NK_ASSERT(expr)
#include "nuklear.h"

static struct nk_context ctx;
static byte memory[1024 * 1024];
static struct nk_user_font font;

static float text_width(nk_handle handle, float height, const char* text,
                        int len)
{
    return len * 8;
}

DEFINE_HOOK(void, UI_Init, (void))
    ORIGINAL(UI_Init)();

    font.userdata.ptr = NULL;
    font.height = 16;
    font.width = text_width;
    nk_init_fixed(&ctx, memory, sizeof(memory), &font);
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
                    UI_DrawRect(r->x, r->y, r->w, r->h, (float*)&color);
                    break;
                }
                case NK_COMMAND_RECT_FILLED: {
                    const struct nk_command_rect_filled* r =
                       (const struct nk_command_rect_filled*)cmd;
                    struct nk_colorf color = nk_color_cf(r->color);
                    UI_FillRect(r->x, r->y, r->w, r->h, (float*)&color);
                    break;
                }
                case NK_COMMAND_TEXT: {
                    const struct nk_command_text* t =
                       (const struct nk_command_text*)cmd;
                    struct nk_colorf color = nk_color_cf(t->foreground);
                    UI_DrawString(t->x, t->y, t->string, UI_SMALLFONT,
                                  (float*)&color);
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
