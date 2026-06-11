#include <q_shared.h>
#include <ui_local.h>
#include "ui.h"

static void WriteTGA(char* filename, const byte* data, int width, int height);

//
// stb_malloc
//
#define size_t unsigned
#define STB_MALLOC_IMPLEMENTATION
#define STBM_ASSERT(expr) \
    (void)(!(expr) ? (Com_Printf("^1%s %d\n", __FILE__, __LINE__), 0) : 0)
#include "stb_malloc.h"

static stbm_heap* heap;
static byte heap_storage[4 * 1024 * 1024];
static size_t heap_offset;
static size_t heap_last_offset;

static void* heap_system_alloc(void* user_context, size_t size_requested,
                               size_t* size_provided)
{
    void* ptr;

    (void)user_context;

    if (heap_offset + size_requested > sizeof(heap_storage)) {
        trap_Error("out of memory");
        return NULL;
    }

    ptr = heap_storage + heap_offset;
    heap_last_offset = heap_offset;
    heap_offset += size_requested;
    *size_provided = size_requested;
    return ptr;
}

static void heap_system_free(void* user_context, void* ptr)
{
    (void)user_context;
    (void)ptr;
    if (ptr == heap_storage + heap_last_offset) {
        Com_Printf("heap_system_free succeeded, freed %d bytes\n",
                   heap_offset - heap_last_offset);
        heap_offset = heap_last_offset;
    } else {
        Com_Printf("heap_system_free failed\n");
    }
}

static void heap_init(void)
{
    static byte storage[0x1000 /*STBM_HEAP_SIZEOF*/];

    stbm_heap_config config;
    memset(&config, 0, sizeof(config));
    config.system_alloc = heap_system_alloc;
    config.system_free = heap_system_free;

    heap = stbm_heap_init(storage, sizeof(storage), &config);
    stbm_heapconfig_gather_full_stats(heap);
}

//
// nuklear
//
double fmod(double x, double y) { return x - ((int)(x / y) * y); }
double acos(double x) { return atan2(sqrt(1 - (x * x)), x); }

#define NK_IMPLEMENTATION
#define NK_ASSERT(expr) \
    (void)(!(expr) ? (Com_Printf("^1%s %d\n", __FILE__, __LINE__), 0) : 0)

#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define STBRP_ASSERT(expr) NK_ASSERT(expr)
#define STBRP_SORT qsort
#define STBTT_assert(expr) NK_ASSERT(expr)
#define STBTT_ifloor nk_ifloorf
#define STBTT_iceil nk_iceilf
#define STBTT_sqrt sqrt
#define STBTT_fmod fmod
#define STBTT_cos nk_cos
#define STBTT_fabs fabs
#define STBTT_pow nk_pow
#define STBTT_acos acos
#define STBTT_memcpy memcpy
#define STBTT_memset memset
#define STBTT_strlen nk_strlen
#include "nuklear.h"

static void* nuklear_alloc(nk_handle userdata, void* old, nk_size size)
{
    (void)userdata;
    (void)old;
    return stbm_alloc(NULL, heap, size, 0);
}

static void nuklear_free(nk_handle userdata, void* old)
{
    (void)userdata;
    stbm_free(NULL, heap, old);
}

static struct nk_allocator nuklear_allocator = {
   0,
   nuklear_alloc,
   nuklear_free,
};

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

    size = trap_FS_FOpenFile("fonts/AdwaitaMono-Italic.ttf", &f, FS_READ);
    font_data = stbm_alloc(NULL, heap, size, 0);
    trap_FS_Read(font_data, size, f);
    trap_FS_FCloseFile(f);

    nk_font_atlas_init(&atlas, &nuklear_allocator);
    nk_font_atlas_begin(&atlas);

    font = nk_font_atlas_add_from_memory(&atlas, font_data, size, 20, NULL);
    stbm_free(NULL, heap, font_data);

    baked = nk_font_atlas_bake(&atlas, &width, &height, NK_FONT_ATLAS_RGBA32);
    WriteTGA("fonts/atlas.tga", baked, width, height);
    shader = trap_R_RegisterShaderNoMip("fonts/atlas");

    nk_font_atlas_end(&atlas, nk_handle_id(shader), &tex_null);
    nk_font_atlas_cleanup(&atlas);

    nk_init(&ctx, &nuklear_allocator, &font->handle);

    nk_buffer_init(&cmds, &nuklear_allocator, 0x100);
    nk_buffer_init(&vbuf, &nuklear_allocator, 0x100);
    nk_buffer_init(&ebuf, &nuklear_allocator, 0x100);

    nk_style_load_all_cursors(&ctx, atlas.cursors);
}

//
// actual shit
//
static refdef_t refdef;

static void refdef_init(void)
{
    float znear = trap_Cvar_VariableValue("r_znear");
    refdef.width = 1600;
    refdef.height = 900;
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
    int c, x, y;

    trap_FS_FOpenFile(filename, &f, FS_WRITE);

    memset(buffer, 0, sizeof(buffer));
    buffer[2] = 2;  // uncompressed type
    buffer[12] = width & 255;
    buffer[13] = width >> 8;
    buffer[14] = height & 255;
    buffer[15] = height >> 8;
    buffer[16] = 32;  // pixel size

    trap_FS_Write(buffer, sizeof(buffer), f);

    c = width * height * 4;
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

    heap_init();
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
    static float value = 0.6f;

    (void)ORIGINAL(UI_Refresh);
    (void)realtime;

    if (!(trap_Key_GetCatcher() & KEYCATCH_UI)) {
        return;
    }

    nk_input_begin(&ctx);
    nk_input_motion(&ctx, mouseX, mouseY);
    nk_input_button(&ctx, NK_BUTTON_LEFT, mouseX, mouseY, mouseDown);
    nk_input_end(&ctx);

    if (
       nk_begin(&ctx, "Show", nk_rect(50, 50, 220, 220),
                NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE |
                   NK_WINDOW_SCALABLE)
    ) {
        nk_layout_row_static(&ctx, 30, 80, 1);
        if (nk_button_label(&ctx, "button")) {
            Com_Printf("stb used: %d stb max used: %d system used: %d\n",
                       heap->cur_outstanding_allocations_bytes,
                       heap->max_outstanding_allocations_bytes, heap_offset);
        }

        nk_layout_row_begin(&ctx, NK_DYNAMIC, 30, 2);
        {
            nk_layout_row_push(&ctx, 0.5);
            nk_label(&ctx, "Volume:", NK_TEXT_LEFT);
            nk_layout_row_push(&ctx, 0.5);
            nk_slider_float(&ctx, 0, &value, 1.0f, 0.1f);
        }
        nk_layout_row_end(&ctx);
    }
    nk_end(&ctx);

    trap_Cvar_SetValue("cg_fov", 90 + (value * 60));

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
        // config.shape_AA = NK_ANTI_ALIASING_ON;
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
