#include "ui.h"

static void WriteTGA(char* filename, const byte* data, int width, int height);

static struct nk_context ctx;

static struct nk_buffer cmds;
static struct nk_buffer vbuf;
static struct nk_buffer ebuf;

static struct nk_font* font;
static struct nk_font_atlas atlas;
static struct nk_draw_null_texture tex_null;

static void nuklear_init(void)
{
    char* font_data;
    int size;
    fileHandle_t f;
    int width, height;
    const void* baked;
    qhandle_t shader;

    size = trap_FS_FOpenFile("fonts/AdwaitaSans-Regular.ttf", &f, FS_READ);
    font_data = malloc(size);
    trap_FS_Read(font_data, size, f);
    trap_FS_FCloseFile(f);

    nk_font_atlas_init_default(&atlas);
    nk_font_atlas_begin(&atlas);

    font = nk_font_atlas_add_from_memory(&atlas, font_data, size, 32, NULL);
    free(font_data);

    baked = nk_font_atlas_bake(&atlas, &width, &height, NK_FONT_ATLAS_RGBA32);
    WriteTGA("fonts/atlas.tga", baked, width, height);
    shader = trap_R_RegisterShaderNoMip("fonts/atlas");

    nk_font_atlas_end(&atlas, nk_handle_id(shader), &tex_null);
    nk_font_atlas_cleanup(&atlas);

    nk_init_default(&ctx, &font->handle);

    nk_buffer_init_default(&cmds);
    nk_buffer_init_default(&vbuf);
    nk_buffer_init_default(&ebuf);

    nk_style_load_all_cursors(&ctx, atlas.cursors);

    ctx.style.window.fixed_background =
       nk_style_item_color(nk_rgba(16, 16, 16, 200));
    ctx.style.window.padding = nk_vec2(20, 20);
}

//
// actual shit
//
static glconfig_t glconfig;
static refdef_t refdef;

static void refdef_init(void)
{
    float znear = trap_Cvar_VariableValue("r_znear");
    refdef.width = glconfig.vidWidth;
    refdef.height = glconfig.vidHeight;
    refdef.fov_x = refdef.fov_y = 90;
    refdef.rdflags = RDF_NOWORLDMODEL;
    refdef.viewaxis[1][0] = -znear * 2 / refdef.width;
    refdef.viewaxis[2][1] = -znear * 2 / refdef.height;
    refdef.viewaxis[0][2] = 1;
    VectorSet(refdef.vieworg, refdef.width / 2.0, refdef.height / 2.0, -znear);
}

static void draw_triangles(int num_triangles, unsigned short* indices,
                           polyVert_t* verts, qhandle_t shader)
{
    polyVert_t tri[3];
    int i, j;

    trap_R_ClearScene();
    for (i = 0; i < num_triangles; i++) {
        for (j = 0; j < 3; j++) {
            tri[j] = verts[*indices++];
            tri[j].xyz[2] = 0;
        }
        trap_R_AddPolyToScene(shader, 3, tri);
    }
    trap_R_RenderScene(&refdef);
}

static void WriteTGA(char* filename, const byte* data, int width, int height)
{
    fileHandle_t f;
    byte buffer[18];
    int x, y;

    trap_FS_FOpenFile(filename, &f, FS_WRITE);

    memset(buffer, 0, sizeof(buffer));
    buffer[2] = 2;  // uncompressed type
    buffer[12] = width & 255;
    buffer[13] = width >> 8;
    buffer[14] = height & 255;
    buffer[15] = height >> 8;
    buffer[16] = 32;  // pixel size

    trap_FS_Write(buffer, sizeof(buffer), f);

    for (y = height - 1; y >= 0; y--) {
        for (x = 0; x < width; x++) {
            int i = ((y * width) + x) * 4;
            // swap rgb to bgr
            trap_FS_Write(&data[i + 2], 1, f);
            trap_FS_Write(&data[i + 1], 1, f);
            trap_FS_Write(&data[i + 0], 1, f);
            trap_FS_Write(&data[i + 3], 1, f);
        }
    }

    trap_FS_FCloseFile(f);
}

DEFINE_HOOK(void, UI_Init, (void))
    ORIGINAL(UI_Init)();

    trap_GetGlconfig(&glconfig);
    nuklear_init();
    refdef_init();
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
    struct nk_rect rect;

    (void)ORIGINAL(UI_Refresh);
    (void)realtime;

    if (!(trap_Key_GetCatcher() & KEYCATCH_UI)) {
        return;
    }

    nk_input_begin(&ctx);
    nk_input_motion(&ctx, mouseX, mouseY);
    nk_input_button(&ctx, NK_BUTTON_LEFT, mouseX, mouseY, mouseDown);
    nk_input_end(&ctx);

    rect.w = glconfig.vidWidth * 0.8;
    rect.h = glconfig.vidHeight * 0.8;
    rect.x = (glconfig.vidWidth - rect.w) * 0.5;
    rect.y = (glconfig.vidHeight - rect.h) * 0.5;

    if (
       nk_begin(&ctx, "Background",
                nk_rect(0, 0, glconfig.vidWidth, glconfig.vidHeight),
                NK_WINDOW_BACKGROUND)
    ) {
        struct nk_command_buffer* canvas = nk_window_get_canvas(&ctx);
        nk_fill_rect(canvas, nk_window_get_bounds(&ctx), 0,
                     nk_rgba(0, 128, 255, 255));
    }
    nk_end(&ctx);

    if (nk_begin(&ctx, "Window", rect, 0)) {
        nk_layout_row_begin(&ctx, NK_DYNAMIC, 0, 3);
        {
            nk_layout_row_push(&ctx, 0.5);
            nk_label(&ctx, "Windowed Mode", NK_TEXT_LEFT);
            nk_layout_row_push(&ctx, 0.25);
            nk_option_label(&ctx, "Windowed", nk_false);
            nk_layout_row_push(&ctx, 0.25);
            nk_option_label(&ctx, "Fullscreen", nk_true);
        }
        nk_layout_row_end(&ctx);

        nk_layout_row_dynamic(&ctx, 0, 2);
        {
            static float value = 0.5;
            nk_label(&ctx, "Volume", NK_TEXT_LEFT);
            nk_slider_float(&ctx, 0.0, &value, 1.0, 0.01);
        }
    }
    nk_end(&ctx);

    {
        static const struct nk_draw_vertex_layout_element vertex_layout[] = {
           {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(polyVert_t, xyz)},
           {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(polyVert_t, st)},
           {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8,
            NK_OFFSETOF(polyVert_t, modulate)},
           {NK_VERTEX_LAYOUT_END}
        };

        const struct nk_draw_command* cmd;
        unsigned short* elems = nk_buffer_memory(&ebuf);

        struct nk_convert_config config;
        memset(&config, 0, sizeof(config));
        config.shape_AA = NK_ANTI_ALIASING_ON;
        config.vertex_layout = vertex_layout;
        config.vertex_size = sizeof(polyVert_t);
        config.vertex_alignment = 4;  // NK_ALIGNOF(polyVert_t);
        config.tex_null = tex_null;
        config.circle_segment_count = 16;
        config.curve_segment_count = 16;
        config.arc_segment_count = 16;
        config.global_alpha = 1.0f;
        nk_convert(&ctx, &cmds, &vbuf, &ebuf, &config);

        nk_draw_foreach(cmd, &ctx, &cmds)
        {
            if (!cmd->elem_count) continue;
            draw_triangles(cmd->elem_count / 3, elems, nk_buffer_memory(&vbuf),
                           cmd->texture.id);
            elems += cmd->elem_count;
        }
        nk_buffer_clear(&cmds);
        nk_buffer_clear(&vbuf);
        nk_buffer_clear(&ebuf);
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
