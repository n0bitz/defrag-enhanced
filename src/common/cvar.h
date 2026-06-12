#ifndef CVAR_HEADER_GUARD__
#define CVAR_HEADER_GUARD__

#include "q_shared.h"

qboolean Cvar_ValidateName(const char* name);
void ParseRGBAf(const char* string, vec4_t rgba);

#endif  // CVAR_HEADER_GUARD__
