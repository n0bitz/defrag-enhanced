#include "fs.h"

// fs.h sets this to trap_FS_Seek_hack, but we need to use the real one.
#undef trap_FS_Seek

void trap_FS_Read(void* buffer, int len, fileHandle_t f);
int trap_FS_Seek(fileHandle_t f, long offset, int origin);

// The official 1.32c quake3.exe supports trap_FS_Seek, but only within the
// first 64K of files in pk3s. This is an attempt to work around that.
int trap_FS_Seek_hack(fileHandle_t f, long offset, int origin)
{
    char buf[0x1000];

    if (origin == FS_SEEK_END) {
        // Not much we can do here, but non-pk3 files will still work.
        return trap_FS_Seek(f, offset, origin);
    }

    if (origin == FS_SEEK_SET) {
        trap_FS_Seek(f, 0, FS_SEEK_SET);
    }

    while (offset > sizeof(buf)) {
        trap_FS_Read(buf, sizeof(buf), f);
        offset -= sizeof(buf);
    }
    trap_FS_Read(buf, offset, f);

    return 0;
}
