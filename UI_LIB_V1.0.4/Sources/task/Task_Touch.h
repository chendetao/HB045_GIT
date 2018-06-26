#ifndef __TASK_TOUCH_H__
#define __TASK_TOUCH_H__

#define TASK_TOUCH_DECTING_EVT 0x00000001
#define TASK_TOUCH_TRIGGER_EVT 0x00000002

extern unsigned long taskTouchTaskId;
extern unsigned long taskTouch( unsigned long task_id, unsigned long events );
extern void taskTouchInit( unsigned long task_id );

#endif
