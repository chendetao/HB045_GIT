#ifndef __TASK_TOUCH_H__
#define __TASK_TOUCH_H__

#define TASK_TOUCH_DECTING_EVT          0x00000001
#define TASK_TOUCH_TRIGGER_EVT          0x00000002
#define TASK_DOUBLE_TOUCH_CLEAR_EVT 	0x00000004//双击清零任务事件

extern unsigned long taskTouchTaskId;


#define TOUCH_FLAG_DOUBLE_TAP  0x01
extern unsigned char touch_flag;
extern unsigned char doubleTouchFlag;//双击标志位 1双击

extern unsigned long taskTouch( unsigned long task_id, unsigned long events );
extern void taskTouchInit( unsigned long task_id );

#endif
