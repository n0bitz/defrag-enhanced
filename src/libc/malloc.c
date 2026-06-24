#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void trap_Error(const char* fmt);

#include "tlsf/tlsf.h"
static tlsf_t tlsf;

static int malloc_initialized;
static char malloc_storage[32 * 1024 * 1024];

static void malloc_init(void)
{
    tlsf = tlsf_create_with_pool(malloc_storage, sizeof(malloc_storage));
    malloc_initialized = 1;
}

void* malloc(size_t size)
{
    void* p;
    if (!malloc_initialized) {
        malloc_init();
    }
    p = tlsf_malloc(tlsf, size);
    if (!p) {
        trap_Error("malloc failed");
    }
    return p;
}

void* realloc(void* p, size_t size)
{
    if (!malloc_initialized) {
        malloc_init();
    }
    p = tlsf_realloc(tlsf, p, size);
    if (!p) {
        trap_Error("realloc failed");
    }
    return p;
}

void free(void* p)
{
    if (!malloc_initialized) {
        malloc_init();
    }
    tlsf_free(tlsf, p);
}

static void malloc_used_walker(void* ptr, size_t size, int used, void* user)
{
    malloc_stats_t* stats = (malloc_stats_t*)user;
    if (used) {
        stats->used += size;
    } else {
        stats->free += size;
    }
    (void)ptr;
}

malloc_stats_t malloc_stats(void)
{
    malloc_stats_t stats = {0};
    if (!malloc_initialized) {
        malloc_init();
    }
    tlsf_walk_pool(tlsf_get_pool(tlsf), malloc_used_walker, &stats);
    return stats;
}
