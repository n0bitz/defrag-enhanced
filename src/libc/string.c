#include "q_shared.h"

int memcmp(const void* ptr1, const void* ptr2, size_t count)
{
    const char* p1 = (char*)ptr1;
    const char* p2 = (char*)ptr2;
    while (count && *p1 == *p2) {
        count--;
        p1++;
        p2++;
    }
    if (count) {
        return *p1 - *p2;
    }
    return 0;
}

char* strdup(const char* string)
{
    size_t size = strlen(string) + 1;
    char* buf = malloc(size);
    if (!buf) {
        return NULL;
    }
    Q_strncpyz(buf, string, size);
    return buf;
}
