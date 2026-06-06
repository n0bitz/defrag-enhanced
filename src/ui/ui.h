#ifndef UI_HEADER_GUARD__
#define UI_HEADER_GUARD__

#include "cvar.h"
#include "hook.h"
#include "ui_local.h"

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#include "nuklear/nuklear.h"

void trap_R_RegisterFont(const char* fontName, int pointSize, fontInfo_t* font);

#endif  // UI_HEADER_GUARD__
