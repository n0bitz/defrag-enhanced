#ifndef FS_HEADER_GUARD__
#define FS_HEADER_GUARD__

#include "q_shared.h"

int trap_FS_Seek_hack(fileHandle_t f, long offset, int origin);
#define trap_FS_Seek trap_FS_Seek_hack

#endif  // FS_HEADER_GUARD__
