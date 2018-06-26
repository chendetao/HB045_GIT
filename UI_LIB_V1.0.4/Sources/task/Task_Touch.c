#include "nrf_gpio.h"

#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_Touch.h"
#include "Task_GUI.h"

#include "Board.h"
#include "Window.h"

unsigned long taskTouchTaskId;
unsigned char keyFlag = 0;

unsigned long taskTouch( unsigned long task_id, unsigned long events )
{
	static unsigned long counter = 0;
	
	if ( events & TASK_TOUCH_DECTING_EVT )
	{
		if ( nrf_gpio_pin_read(BUTTON_PIN) == 0 )
		{
			counter++; win_ticks = 0;
			
			if ( counter > 24 )
			{
				osal_set_event( taskGUITaskId, TASK_GUI_PRESS_EVT );
				return ( events ^ TASK_TOUCH_DECTING_EVT );
			}
			
			osal_start_timerEx( task_id,TASK_TOUCH_DECTING_EVT, 50 );		
			
			keyFlag = 1;
		}else{
		
			if ( counter > 24 )
			{
				osal_set_event( taskGUITaskId, TASK_GUI_PRESS_EVT );
			}else{
				osal_set_event( taskGUITaskId, TASK_GUI_TOUCH_EVT );
			}	

			keyFlag = 0;
		}
		
		return ( events ^ TASK_TOUCH_DECTING_EVT );
	}
	
	if ( events & TASK_TOUCH_TRIGGER_EVT )
	{
		if ( nrf_gpio_pin_read(BUTTON_PIN) == 0 )
		{
			osal_start_timerEx( task_id,TASK_TOUCH_DECTING_EVT, 1 ); 
			
			counter = 0;
		}
		
		return ( events ^ TASK_TOUCH_TRIGGER_EVT );
	}
	
	return 0;
}

void taskTouchInit( unsigned long task_id )
{
	taskTouchTaskId = task_id;
}

