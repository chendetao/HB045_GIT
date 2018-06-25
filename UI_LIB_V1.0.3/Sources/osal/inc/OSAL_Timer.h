
#ifndef __OSAL_TIMER_H__
#define __OSAL_TIMER_H__

#include <stdbool.h>

#define OSAL_TIMER_MAX  40


#define OSAL_TICK_PRIOR                  (1000/25)          ( OSAL frecenly: Hz)

typedef void (*timer_handler)(void *p_context);

typedef struct {
    unsigned long timerId;
	unsigned long taskId;
	unsigned long event;
	unsigned long timeout_value;
    timer_handler handler;
    bool  run;
}Timer;

extern unsigned long osal_systemClock;
extern unsigned long systemTick;

extern void osalTimerInit( void );
extern void osal_timer_start(void);
extern Timer timer[OSAL_TIMER_MAX];

extern int osal_create_timer(unsigned long *timer_id, unsigned long mode,\
            timer_handler handler);
extern int osal_start_timer(unsigned long timer_id, unsigned long timeout);
extern int osal_stop_timer(unsigned long timer_id);
extern int osal_release_timer(unsigned long timer_id );

extern unsigned long osal_start_timerEx( unsigned long task_id, unsigned long event_id,\
				long timeout_value);
extern unsigned long osal_stop_timerEx( unsigned long task_id, unsigned long event_id);

extern void osal_timer_isr( unsigned long elapsedTick );

/*****************************************************************
 心率相关定时器操作
 ****************************************************************/


extern void hr_clock_rtc_init(void);
extern void hr_clock_rtc_start(void);
extern void hr_clock_rtc_stop(void);

extern volatile unsigned long hr_mticks;

#endif
