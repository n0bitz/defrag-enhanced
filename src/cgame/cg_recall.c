#include "cgame.h"

#define RECALL_FILE_PATH "temp/current.vhs"

typedef struct {
    int index;
    int length;
    saveState_t buffer[60 * 1000 / 8];
} recallBuffer_t;

static recallBuffer_t recall_states;
// KTODO: make these cvars?
static qboolean recalling, forwarding, rewinding;
static int recall_frame_idx = 0;

static void recall_seek(int base_idx, int offset)
{
    recall_frame_idx = (base_idx + offset) % recall_states.length;
    if (recall_frame_idx < 0) {
        recall_frame_idx += base_idx + recall_states.length;
    }
}

// KTODO: call lazily
static void InitRecallBuffer(void)
{
    fileHandle_t recall_file;

    memset(&recall_states, 0, sizeof(recall_states));
    trap_FS_FOpenFile(RECALL_FILE_PATH, &recall_file, FS_READ);
    if (recall_file) {
        trap_FS_Read(&recall_states, sizeof(recall_states), recall_file);
        trap_FS_FCloseFile(recall_file);
    }
}

void CG_AddRecallState(void)
{
    if (recalling) {
        return;
    }

    if (!CaptureCurrentState(&recall_states.buffer[recall_states.index])) {
        return;
    }

    if (recall_states.length < ARRAY_LENGTH(recall_states.buffer)) {
        recall_states.length += 1;
    }
    recall_states.index =
       (recall_states.index + 1) % ARRAY_LENGTH(recall_states.buffer);
}

// KTODO: call
void CG_SaveRecallBuffer(void)
{
    fileHandle_t recall_file;

    trap_FS_FOpenFile(RECALL_FILE_PATH, &recall_file, FS_WRITE);
    if (!recall_file) {
        Com_Printf(LOG_ERROR "couldn't write to recall file\n");
        return;
    }
    trap_FS_Write(&recall_states, sizeof(recall_states), recall_file);
    trap_FS_FCloseFile(recall_file);
}

void CG_DrawRecall(void)
{
    refdef_t refdef;

    if (!recalling) {
        return;
    }

    if (forwarding) {
        recall_seek(recall_frame_idx, +1);
    }
    if (rewinding) {
        recall_seek(recall_frame_idx, -1);
    }

    memset(&refdef, 0, sizeof(refdef));
    AxisClear(refdef.viewaxis);
    AnglesToAxis(recall_states.buffer[recall_frame_idx].viewangles,
                 refdef.viewaxis);
    // TODO: fix fov stuff since player can be in water or whatever when
    // recalling a position not in water or something
    refdef.fov_x = cg.refdef.fov_x;
    refdef.fov_y = cg.refdef.fov_y;
    // KTODO: customizable?
    refdef.x = 0;
    refdef.y = 0;
    refdef.width = 300;
    refdef.height = 300;
    refdef.time = cg.time;
    VectorCopy(recall_states.buffer[recall_frame_idx].origin, refdef.vieworg);
    trap_R_RenderScene(&refdef);
}

consoleCommandStatus_t CG_Recall_f(void)
{
    recalling = !recalling;
    recall_seek(recall_states.index, -1);
    Com_Printf(LOG_INFO "Recall (%s)\n", (recalling) ? "ON" : "OFF");
    return CON_CMD_HANDLED;
}

consoleCommandStatus_t IN_RecallForwardDown(void)
{
    forwarding = qtrue;
    return CON_CMD_HANDLED;
}

consoleCommandStatus_t IN_RecallForwardUp(void)
{
    forwarding = qfalse;
    return CON_CMD_HANDLED;
}

consoleCommandStatus_t IN_RecallRewindDown(void)
{
    rewinding = qtrue;
    return CON_CMD_HANDLED;
}

consoleCommandStatus_t IN_RecallRewindUp(void)
{
    rewinding = qfalse;
    return CON_CMD_HANDLED;
}
