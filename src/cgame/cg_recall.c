#include "cgame.h"

#define RECALL_CAPACITY (60 * 1000 / 8)  // 1 min worth at 125fps
#define RECALL_FILE_PATH "temp/current.cd_01"

typedef struct {
    int write_idx;  // index to append to
    int length;
    saveState_t states[RECALL_CAPACITY];
} recallRingBuffer_t;

static recallRingBuffer_t history;

static struct {
    qboolean visible;
    int cursor;
    qboolean scrubbing_forward;
    qboolean scrubbing_backward;
} viewer;

static void ScrubViewer(int offset)
{
    int length = history.length;

    if (length <= 0) {
        return;
    }

    viewer.cursor = (viewer.cursor + (offset % length) + length) % length;
}

// KTODO: call lazily
static void InitRecallBuffer(void)
{
    fileHandle_t recall_file;

    memset(&history, 0, sizeof(history));
    trap_FS_FOpenFile(RECALL_FILE_PATH, &recall_file, FS_READ);
    if (recall_file) {
        trap_FS_Read(&history, sizeof(history), recall_file);
        trap_FS_FCloseFile(recall_file);
    }
}

void CG_AddRecallState(void)
{
    if (viewer.visible) {
        return;
    }

    if (!CaptureCurrentState(&history.states[history.write_idx])) {
        return;
    }

    if (history.length < ARRAY_LENGTH(history.states)) {
        history.length += 1;
    }
    history.write_idx = (history.write_idx + 1) % ARRAY_LENGTH(history.states);
}

saveState_t* CG_GetRecallState(void)
{
    // KTODO: ring buffer length check probably not needed
    if (!viewer.visible || history.length <= 0) {
        return NULL;
    }

    return &history.states[viewer.cursor];
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
    // KTODO: write can fail?
    trap_FS_Write(&history, sizeof(history), recall_file);
    trap_FS_FCloseFile(recall_file);
}

void CG_DrawRecall(void)
{
    refdef_t refdef;

    if (!viewer.visible) {
        return;
    }

    if (viewer.scrubbing_forward) {
        ScrubViewer(+1);
    }
    if (viewer.scrubbing_backward) {
        ScrubViewer(-1);
    }

    memset(&refdef, 0, sizeof(refdef));
    AxisClear(refdef.viewaxis);
    AnglesToAxis(history.states[viewer.cursor].viewangles, refdef.viewaxis);
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
    VectorCopy(history.states[viewer.cursor].origin, refdef.vieworg);
    trap_R_RenderScene(&refdef);
}

consoleCommandStatus_t CG_Recall_f(void)
{
    if (!viewer.visible && history.length <= 0) {
        Com_Printf(LOG_ERROR "Nothing to recall\n");
        return CON_CMD_HANDLED;
    }

    viewer.visible = !viewer.visible;
    if (viewer.visible) {
        // KTODO: hack to go to most recent frame
        viewer.cursor = 0;
        ScrubViewer(-1);
    }

    Com_Printf(LOG_INFO "Recall (%s)\n", (viewer.visible) ? "ON" : "OFF");
    return CON_CMD_HANDLED;
}

consoleCommandStatus_t IN_RecallForwardDown(void)
{
    viewer.scrubbing_forward = qtrue;
    return CON_CMD_HANDLED;
}

consoleCommandStatus_t IN_RecallForwardUp(void)
{
    viewer.scrubbing_forward = qfalse;
    return CON_CMD_HANDLED;
}

consoleCommandStatus_t IN_RecallRewindDown(void)
{
    viewer.scrubbing_backward = qtrue;
    return CON_CMD_HANDLED;
}

consoleCommandStatus_t IN_RecallRewindUp(void)
{
    viewer.scrubbing_backward = qfalse;
    return CON_CMD_HANDLED;
}
