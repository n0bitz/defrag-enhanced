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
        if (c == '\\' || c == '\"' || c == ';' || c == '%' || c <= ' ' ||
            c >= '~') {
            return qfalse;
        }
    }

    if ((s - name) >= MAX_STRING_CHARS) {
        return qfalse;
    }

    return qtrue;
}
