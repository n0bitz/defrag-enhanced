#include "cgame.h"

static char memoryPool[512 * 1024];
static int allocPoint;

void* CG_Alloc(int size)
{
    char* p = &memoryPool[allocPoint];
    if (allocPoint + size > sizeof(memoryPool)) {
        static qboolean warned;
        if (!warned) {
            Com_Printf(LOG_WARN "cgame pool exhausted\n");
            warned = qtrue;
        }
        return NULL;
    }
    allocPoint += (size + 31) & ~31;
    return p;
}

char* CG_strdup(const char* string)
{
    int size = strlen(string) + 1;
    char* buf = CG_Alloc(size);
    if (!buf) {
        return "";
    }
    Q_strncpyz(buf, string, size);
    return buf;
}
