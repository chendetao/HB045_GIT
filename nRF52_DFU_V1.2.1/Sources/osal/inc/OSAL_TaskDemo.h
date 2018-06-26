#ifndef __TASKDEMO_H__
#define __TASKDEMO_H__

#define DEMO_START_EVENT		0x00000001
#define DEMO_STOP_EVENT         0x00000002

extern unsigned long taskDemo( unsigned long task_id, unsigned long events );
extern void taskDemoInit( unsigned long task_id );

#endif
