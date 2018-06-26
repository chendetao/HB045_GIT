/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : OSAL.c                                                            *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : OSAL core function implementation.                              *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017-3.29  1.0        build this moudle                   *
*                                                                              *
*                                                                              *
* Created : 2017-3-29                                                          *
* Last modified : 2017.3.29                                                    *
*******************************************************************************/

#include "OSAL.h"
#include "OSAL_Task.h"
#include "OSAL_PwrMgr.h"
#include "OSAL_Timer.h"
#include "OSAL_ASSERT.h"

unsigned long sleepEnable = 1;

unsigned long osal_set_event( unsigned long task_id, unsigned long event_flag)
{
	if ( task_id < tasksCnt )
	{
		unsigned long x = 0;
		/* Entry critical */
		
		entry_critical(x);
		
		taskEventsTable[task_id] |= event_flag; /* Set the event bit(s) */
		/* Leave critical */
		
		leave_critical(x);
		
		return ( 0 );
	}
	else
	{
#if defined(_OSAL_DEBUG)
        OSAL_ASSERT(1);        
#else
		return ( 1 );
#endif
	}
}

unsigned long osal_clear_event( unsigned long task_id, unsigned long event_flag)
{
      if ( task_id < tasksCnt )
      {
            unsigned long x = 0;
              
            /* Entry critical */
            entry_critical(x);  
              
            taskEventsTable[task_id] &= ~(event_flag); /* Clear the event bit(s) */
              
            /* Leave critical */
              
            leave_critical(x);
              
            return ( 0 );
      }
       else
      {
#if defined(_OSAL_DEBUG)
        OSAL_ASSERT(1);        
#else
		return ( 1 );
#endif
      }
}

/**
 * Operating System Abstraction Layer Entry.
 */
void osal_start_system(void)
{
   osal_timer_start();
}

void osal_run_system ( void )
{
	int idx = 0;
	unsigned long x = 0;
	
	/**
	 * Find the task that has The highest priority event 
	 */
	do{	
		if ( taskEventsTable[idx] ){
			break;
		}
		
	} while (++idx < tasksCnt);
	
	if ( idx < tasksCnt )
	{
		/* If A task has event flag, we call the handler */
		unsigned long events;
		
		/* Entry critical */
		entry_critical(x);
		
		events = taskEventsTable[idx];
		taskEventsTable[idx] = 0;
		
		/* Leave critical */
		leave_critical(x);
		
		/* Call the event handler */
		events = (tasksArr[idx])(idx, events);
		
		/* Entry critical */
		entry_critical(x);
		
		taskEventsTable[idx] |= events;
		
		/* Leave critical */
		leave_critical(x);
		
	}else{	
        
		/* If nothing to do , we go to poserconserve */
        
		if ( sleepEnable == 1 )
		{
			osal_pwrmgr_powerconserve();
		}
	}
}
