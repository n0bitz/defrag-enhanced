#ifndef EXTENSIONS_HEADER_GUARD__
#define EXTENSIONS_HEADER_GUARD__

#include "q_shared.h"

typedef void (*extensionTrap_t)(void);

qboolean GetExtension(const char* name, extensionTrap_t* trap);

qboolean trap_GetValue(char* value, int valueSize, const char* key);
void trap_Cvar_SetDescription_Q3E(const char* name, const char* description);

#endif  // EXTENSIONS_HEADER_GUARD__
