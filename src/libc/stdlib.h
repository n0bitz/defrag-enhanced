#ifndef STDLIB_HEADER_GUARD__
#define STDLIB_HEADER_GUARD__

#include <stddef.h>

int abs(int n);
double atof(const char* string);
int atoi(const char* string);
int rand(void);
void srand(unsigned seed);

void* malloc(size_t size);
void* realloc(void* p, size_t size);
void free(void* p);

typedef struct {
    size_t used;
    size_t free;
} malloc_stats_t;

malloc_stats_t malloc_stats(void);

typedef int cmp_t(const void*, const void*);
void qsort(void* a, size_t n, size_t es, cmp_t* cmp);

#endif  // STDLIB_HEADER_GUARD__
