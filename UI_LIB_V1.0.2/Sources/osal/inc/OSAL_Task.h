
#ifndef __OSAL_TASK_H__
#define __OSAL_TASK_H__

typedef unsigned long (*pTaskEventHandlerFn)(unsigned long task_id,\
            unsigned long event);

extern const pTaskEventHandlerFn tasksArr[];
extern unsigned long taskEventsTable[];
extern const unsigned long tasksCnt;

extern void osal_init_tasks( void );
extern unsigned long osal_event_count( void );

#endif

