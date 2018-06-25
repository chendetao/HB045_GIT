//#include <string.h>
//#include <stdbool.h>
//#include "OSAL.h"
//#include "OSAL_Timer.h"
//#include "Task_Step.h"
//#include "SleepMonitor.h"
//#include "ClockTick.h"
//#include "UserSetting.h"

//#define 	ACTION_ARRAY_NR   20      /* 20 minutes */

//#define TO_MINUTES(x) ((x)/60)
//#define D_TIMES(a,b) ((TO_MINUTES((a)))-(TO_MINUTES((b))))

//unsigned char tick = 0;
//unsigned char action_array[ACTION_ARRAY_NR];
//float pA,pB;
//int cA,cB;

//void action_clear(void)
//{
//	memset( action_array, 0, sizeof(action_array));
//}

//void action_put( unsigned char action)
//{	
//	for ( int i = 0; i < (ACTION_ARRAY_NR-1); i++ )
//	{
//		action_array[i] = action_array[i+1];
//	}
//	
//	action_array[ACTION_ARRAY_NR-1] = action;
//	
//	cA = cB = 0;
//	
//	for ( int i = 0; i < ACTION_ARRAY_NR; i++ )
//	{
//		if ( action_array[i] == 1 )
//		{
//			if ( i < ACTION_ARRAY_NR/2 ) 
//			{
//				cA++;
//			}else
//			{
//				cB++;
//			}
//		}
//	}	
//	
//	pA = cA/(ACTION_ARRAY_NR/2.0f);
//	pB = cB/(ACTION_ARRAY_NR/2.0f);
//}

//long start_time, end_time;
//int light_sleep_cnt = 0, deep_sleep_cnt = 0;

//int call_cnt = 0;

//unsigned char sleep_status_buffer[20];

//unsigned char *update_sleep_status_buffer(void)
//{
//	sleep_status_buffer[0] = 0x0F;
//	sleep_status_buffer[1] = 0x10;
//	sleep_status_buffer[2] = config.sleep_mode & 0xFF;
//	sleep_status_buffer[3] = cA & 0xFF;
//	sleep_status_buffer[4] = cB & 0xFF;
//	sleep_status_buffer[5] = ((int)(pA * 100)) & 0xFF;
//	sleep_status_buffer[6] = ((int)(pB * 100)) & 0xFF;
//	
//	return sleep_status_buffer;
//}

//void sleep_put( long end_time, long start_time, int flag)
//{
//	#if 0
//	int d_minutes = D_TIMES(end_time, start_time);
//	if ( flag == 1 )
//	{
//		sleep.deep_time += d_minutes;
//		sleep.deep_time_all += d_minutes;
//	}else{
//		sleep.light_time += d_minutes;
//		sleep.light_time_all += d_minutes;
//	}
//	#endif
//}

//void sleep_counter(void)
//{
//	call_cnt++;
//	
//	/** pB > 0.00000001 ---> pB > 0 : 过滤突然静止放置的情形 */

//	if ( ( pA  > pB ) && (pB <= 0.40f) && (pB > 0.0f) 
//		&& config.sleep_mode == 0 ) /** 清醒-->浅睡˯ */
//	{
//		config.sleep_mode = 1;
//		start_time = systemTick;
//		
//		memcpy( action_array, 
//				action_array+ACTION_ARRAY_NR/2,
//			    ACTION_ARRAY_NR/2);
//		
//		osal_set_event( taskStepTaskId, TASK_STEP_SLEEP_START_EVT);
//		
//		return;
//	}
//	
//	if ( (pB > pA) && (pB > 0.50f) && (config.sleep_mode == 1) )  /** 浅睡-->清醒 */
//	{
//		end_time = systemTick;
//		/** 
//		* 浅睡有效条件:
//		* 1 浅睡时间>=10分钟有效
//		* 2 浅睡时间<10分钟、>1分钟,且前一个模式是深睡
//		*/
//		
//		if ( (D_TIMES(end_time, start_time) >= 10) || 
//			(( D_TIMES(end_time, start_time) >= 1 )
//            && ( D_TIMES(end_time, start_time) < 10 )		
//			&& ( config.last_sleep_mode == 2 ) ) )
//		{
//			sleep_put( end_time, start_time, 0);
//			light_sleep_cnt++;
//	
//			config.last_sleep_mode = config.sleep_mode = 0;
//			
//			osal_set_event( taskStepTaskId, TASK_STEP_SLEEP_DONE_EVT);			
//		}
//		
//		return;
//	}
//	
//	if ( pB <= 0.10f && config.sleep_mode == 1 )
//		/** 浅睡-->深睡˯ */
//	{
//		end_time = systemTick;
//		
//		if ( D_TIMES(end_time, start_time) >= 10 )
//		{
//			/** 浅睡时间>=10分钟有效 */
//			config.sleep_mode = 2;			
//			end_time = systemTick;
//			
//			sleep_put( end_time, start_time, 0);
//			
//			/** 深睡开始 */
//			start_time = systemTick;
//			
//			/** 保存当前时段睡眠数据,并开始下一段计时 */
//			osal_set_event( taskStepTaskId, TASK_STEP_SLEEP_DONE_EVT);			
//			osal_start_timerEx( taskStepTaskId, TASK_STEP_SLEEP_START_EVT, 6000);			
//		}	
//		
//		return;
//	}
//	
//	if ( config.sleep_mode == 2 && pB > 0.10f )
//	{
//		/** 深睡-->浅睡 */
//		
//		end_time = systemTick;
//		
//		if ( D_TIMES(end_time,start_time) >= (60*2.5) )
//		{
//			// >= 2.5h 无效
//			config.sleep_mode = 0;
//			return;
//		}else
//		if ( D_TIMES(end_time,start_time) >= 7 )
//		{
//			// >=7分钟，深睡有效,退出深睡进入浅睡时,
//			// 复位浅睡时间计数器.
//			sleep_put( end_time, start_time, 1);
//			
//			start_time = systemTick;
//			
//			osal_set_event( taskStepTaskId, TASK_STEP_SLEEP_DONE_EVT);			
//			osal_start_timerEx( taskStepTaskId, TASK_STEP_SLEEP_START_EVT, 6000);			
//		}else{
//			// <7分钟，深睡无效,计入浅睡.
//		}
//		
//		config.last_sleep_mode = config.sleep_mode;
//		config.sleep_mode = 1;	
//		
//		return;
//	}
//}
