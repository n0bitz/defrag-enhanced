#include "cgame.h"

DEFINE_HOOK(void, CG_TransitionSnapshot, (void))
    ORIGINAL(CG_TransitionSnapshot)();
    CG_AddRecallState();
END_HOOK
