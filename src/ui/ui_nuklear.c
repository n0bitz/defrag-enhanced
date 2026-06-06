#include "ui.h"

// TODO put stuff in libc
double fmod(double x, double y) { return x - ((int)(x / y) * y); }
double acos(double x) { return atan2(sqrt(1 - (x * x)), x); }

#define NK_ASSERT(expr) \
    (void)(!(expr) ? (Com_Printf("^1%s %d\n", __FILE__, __LINE__), 0) : 0)
#define NK_MEMSET memset
#define NK_MEMCPY memcpy

#define STBTT_sqrt sqrt
#define STBTT_pow nk_pow

#define NK_IMPLEMENTATION
#include "nuklear/nuklear.h"
