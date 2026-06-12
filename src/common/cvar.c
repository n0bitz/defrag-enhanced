#include "cvar.h"

// From Quake3e
qboolean Cvar_ValidateName(const char* name)
{
    const char* s;
    char c;

    if (!name) {
        return qfalse;
    }

    s = name;
    while ((c = *s++) != '\0') {
        if (
           c == '\\' || c == '\"' || c == ';' || c == '%' || c <= ' ' ||
           c >= '~'
        ) {
            return qfalse;
        }
    }

    if ((s - name) >= MAX_STRING_CHARS) {
        return qfalse;
    }

    return qtrue;
}

void ParseRGBAf(const char* string, vec4_t rgba)
{
    sscanf(string, "%f %f %f %f", &rgba[0], &rgba[1], &rgba[2], &rgba[3]);
}
