#ifndef __TASK_FRAME_H__
#define __TASK_FRAME_H__

#define FRAME_TASK_SYNC_FRAME_EVT_0         0x00000001
#define FRAME_TASK_SYNC_FRAME_EVT_1         0x00000002
#define FRAME_TASK_SYNC_FRAME_EVT_2         0x00000004
#define FRAME_TASK_SYNC_FRAME_EVT_3         0x00000008
#define FRAME_TASK_SYNC_FRAME_EVT_4         0x00000010
#define FRAME_TASK_SYNC_FRAME_EVT_5         0x00000020
#define FRAME_TASK_SYNC_FRAME_EVT_6         0x00000040
#define FRAME_TASK_SYNC_FRAME_EVT_7         0x00000080


extern unsigned long taskFrameTaskId;

extern void taskFrameInit(unsigned long task_id);
extern unsigned long taskFrame( unsigned long task_id, unsigned long events );

#endif
