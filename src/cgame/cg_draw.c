#include "cgame.h"

/*
=================
CG_Draw2D_df
=================
*/
DEFINE_HOOK(void CG_Draw2D_df, (stereoFrame_t stereoFrame))
{
    CG_AddEntityPOIs();
    CG_DrawPOIs();
    ORIGINAL(CG_Draw2D_df)(stereoFrame);
}

/*
=================
CG_DrawActive
=================
*/
DEFINE_HOOK(void CG_DrawActive, (stereoFrame_t stereoFrame))
{
    CG_DrawEntityConnections();
    ORIGINAL(CG_DrawActive)(stereoFrame);
}
