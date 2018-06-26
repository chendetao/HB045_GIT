#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_Motor.h"
#include "Board.h"
#include "UserSetting.h"

void motor_disable(void)
{
	MOTOR_ENABLE(false);
}

unsigned long taskMotorTaskId;

unsigned long taskMotor( unsigned long task_id, unsigned long events )
{
	static unsigned char viborate_flag = 0, cnt = 0;
	
	if ( events & TASK_MOTOR_ACTION_NOTIFY_EVT )
	{		
		if ( cnt++ < 6 )
		{
			if ( viborate_flag == 0 )
			{
				MOTOR_ENABLE(true);
				viborate_flag = 1;
				osal_start_timerEx( task_id, TASK_MOTOR_ACTION_NOTIFY_EVT, 100);
			}else{
 				MOTOR_ENABLE(false);
				viborate_flag = 0;
				osal_start_timerEx( task_id, TASK_MOTOR_ACTION_NOTIFY_EVT, 50);
			}				
		}else{
			MOTOR_ENABLE(false);
			
			cnt = 0; viborate_flag = 0;
		}
		
		return ( events ^ TASK_MOTOR_ACTION_NOTIFY_EVT );
	}
	
	if ( events & TASK_MOTOR_ACTION_APP_MSG_EVT )
	{		
		if ( viborate_flag == 0 )
		{
			MOTOR_ENABLE(true);
			viborate_flag = 1;
			osal_start_timerEx( task_id, TASK_MOTOR_ACTION_APP_MSG_EVT, 200);
		}else{
			MOTOR_ENABLE(false);
			viborate_flag = 0;
		}				
		
		return ( events ^ TASK_MOTOR_ACTION_APP_MSG_EVT );
	}	
	
	if ( events & TASK_MOTOR_BLE_CONNECT_EVT )
	{	
		if ( viborate_flag == 0 )
		{
			viborate_flag = 1;
			
			MOTOR_ENABLE(true);
			
			osal_start_timerEx( task_id, TASK_MOTOR_BLE_CONNECT_EVT, 100);
		}else
		{
			viborate_flag = 0;
			MOTOR_ENABLE(false);
		}
			
		return ( events ^ TASK_MOTOR_BLE_CONNECT_EVT );
	}
	
	if ( events & TASK_MOTOR_ACTION_DONE_EVT )
	{	
		if ( viborate_flag == 0 )
		{
			viborate_flag = 1;
			
			MOTOR_ENABLE(true);
			
			osal_start_timerEx( task_id, TASK_MOTOR_ACTION_DONE_EVT, 250);
		}else
		{
			viborate_flag = 0;
			MOTOR_ENABLE(false);
		}
		
		return ( events ^ TASK_MOTOR_ACTION_DONE_EVT );
	}
	
	if ( events & TASK_MOTOR_BMA_DONE_EVT )
	{	
		if ( viborate_flag == 0 )
		{
			viborate_flag = 1;
			
			MOTOR_ENABLE(true);
			
			osal_start_timerEx( task_id, TASK_MOTOR_BMA_DONE_EVT, 250);
		}else
		{
			viborate_flag = 0;
			MOTOR_ENABLE(false);
		}
		
		return ( events ^ TASK_MOTOR_BMA_DONE_EVT );
	}	
	
	if ( events & TASK_MOTOR_VIBORATE_EVT )
	{
		osal_set_event ( task_id, TASK_MOTOR_BLE_CONNECT_EVT );
		
		return ( events ^ TASK_MOTOR_VIBORATE_EVT );
	}
	
	if( events & TASK_MOTOR_TEST_EVT )
	{ 
		if ( nrf_gpio_pin_read(BAT_CHARGE_PIN) == 0 )
		{
			osal_set_event ( task_id, TASK_MOTOR_ACTION_DONE_EVT );
		}else{
			config.motor_test_enable = 0;
		}
		
		return (events ^ TASK_MOTOR_TEST_EVT);
	}

//	if ( events & TASK_MOTOR_APP_SEND_EVT )
//	{	
//		if ( viborate_flag == 0 )
//		{
//			viborate_flag = 1;
//			
//			MOTOR_ENABLE(true);
//			
//			osal_start_timerEx( task_id, TASK_MOTOR_APP_SEND_EVT, 500);
//		}else
//		{
//			viborate_flag = 0;
//			MOTOR_ENABLE(false);
//		}
//			
//		return ( events ^ TASK_MOTOR_APP_SEND_EVT );
//	}
	
	return 0;
}

void taskMotorInit( unsigned long task_id )
{
	taskMotorTaskId = task_id;
	
	MOTOR_INIT();
	
	osal_set_event ( task_id, TASK_MOTOR_TEST_EVT );
}
