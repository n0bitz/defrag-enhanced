#ifndef STRING_HEADER_GUARD__
#define STRING_HEADER_GUARD__

#include <stddef.h>

void* memmove(void* dest, const void* src, size_t count);
void* memset(void* dest, int c, size_t count);
void* memcpy(void* dest, const void* src, size_t count);
int memcmp(const void* ptr1, const void* ptr2, size_t count);
size_t strlen(const char* string);
char* strcat(char* strDestination, const char* strSource);
char* strcpy(char* strDestination, const char* strSource);
int strcmp(const char* string1, const char* string2);
char* strchr(const char* string, int c);
char* strdup(const char* string);
char* strstr(const char* string, const char* strCharSet);
char* strncpy(char* strDest, const char* strSource, size_t count);

#endif  // STRING_HEADER_GUARD__
