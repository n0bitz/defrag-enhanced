#ifndef STDIO_HEADER_GUARD__
#define STDIO_HEADER_GUARD__

#include <stdarg.h>
#include <stddef.h>

#define printf Com_Printf

int vsprintf(char* buffer, const char* fmt, va_list argptr);
int sscanf(const char* buffer, const char* fmt, ...);

#endif  // STDIO_HEADER_GUARD__
