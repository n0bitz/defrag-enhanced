#include "cgame.h"

/*
===============
CG_AddCEntity
===============
*/
DEFINE_HOOK(void, CG_AddCEntity, (centity_t* cent))
    CG_AddCEntityPOI(cent);
    ORIGINAL(CG_AddCEntity)(cent);
END_HOOK
