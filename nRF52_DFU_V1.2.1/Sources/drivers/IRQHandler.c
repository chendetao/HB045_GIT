#include "nrf_gpio.h"

#include "OSAL.h"
#include "Task_Touch.h"
#include "Task_Step.h"
#include "Task_Battery.h"
#include "OSAL_Timer.h"

volatile unsigned long taskStepInterrutCount = 0;
volatile unsigned long taskStepInterrutMonotorCnt = 0;
volatile unsigned long touchInterruptCount = 0;

void GPIOTE_IRQHandler(void)
{	
	if ( NRF_GPIOTE->EVENTS_IN[0] == 1 )
	{
		NRF_GPIOTE->EVENTS_IN[0] = 0;
		(void)NRF_GPIOTE->EVENTS_IN[0]; /* Flush write cache */

		osal_set_event( taskStepTaskId, STEP_TASK_DECT_EVT);	

		taskStepInterrutCount++;
	}
	
	if ( NRF_GPIOTE->EVENTS_IN[1] == 1 )
	{
		NRF_GPIOTE->EVENTS_IN[1] = 0;	
		(void)NRF_GPIOTE->EVENTS_IN[1]; /* Flush write cache */
		
		osal_set_event( taskTouchTaskId, TASK_TOUCH_TRIGGER_EVT );	

		touchInterruptCount++;
	}
	
	if ( NRF_GPIOTE->EVENTS_IN[2] == 1 )
	{
		NRF_GPIOTE->EVENTS_IN[2] = 0;
		(void)NRF_GPIOTE->EVENTS_IN[2]; /* Flush write cache */
		
		osal_set_event( taskBatteryTaskId, TASK_BATTERY_CHARGE_EVT );
	}
	
	if ( NRF_GPIOTE->EVENTS_IN[3] == 1 )
	{
 		NRF_GPIOTE->EVENTS_IN[3] = 0;
		(void)NRF_GPIOTE->EVENTS_IN[3]; /* Flush write cache */
		
		osal_set_event( taskBatteryTaskId, TASK_BATTERY_CHARGE_EVT );
	}	
}
