#include <stdlib.h>
#include "OSAL.h"
#include "OSAL_Timer.h"
#include "OSAL_Task.h"
#include "Window.h"

#include "Task_Hr.h"
#include "Task_Upload.h"
#include "Task_Motor.h" 
#include "Task_Store.h"
#include "UserSetting.h"
#include "Task_Flash.h"
#include "Task_Battery.h"
#include "HeartComp.h"
#include "sleep_algorithm.h"

#include "Board.h"
#include "nrf_saadc.h"
#include "nrf_gpio.h"
#include "Debug.h"
#include "FFT.h"

void hr_pwr_enable( bool enable);

void hr_timer_disable(void)
{
	hr_clock_rtc_stop();
	hr_pwr_enable(false);
	HR_ADC_PIN_OUTPUT_LOW();
	nrf_saadc_disable();
	
	sleepEnable = 1;	
}

nrf_saadc_value_t  saadc_buff1[4];


/* 
	RESULT = [V(P) - V(N) ] * GAIN/REFERENCE * 2(RESOLUTION - m)
	V(P)
		is the voltage at input P
	V(N)
		is the voltage at input N
	GAIN
		is the selected gain setting
	REFERENCE
		is the selected reference voltage

	and m=0 if CONFIG.MODE=SE, or m=1 if CONFIG.MODE=Diff

	2^14 = 16383
*/

unsigned long taskHrTaskId;
short current_hr_adc, current_touch_adc;
int to_close;

bool leaveEnable = false;
volatile unsigned long dect_mTicks = 0, need_mTicks = 0;
volatile unsigned char last_hr = 0, hr_showFlag = 0;

void hr_callback(void)
{		
	
}
	
short leave_array[62]; // 2.5s (2500/25 = 62.5)

void leave_array_put( short val )
{
	for ( int i = 0; i < SIZEOF(leave_array)-1; i++ )
	{
		leave_array[i] = leave_array[i+1];
	}
	
	leave_array[SIZEOF(leave_array)-1] = val;
}

unsigned char is_leave_true = 0;

bool is_leave(void)
{
	int max = 0, min = 16384;
	
	for ( int i = 0; i < SIZEOF(leave_array); i++ )
	{
		if ( max < leave_array[i] )
		{
			max = leave_array[i];
		}
		
		if ( min > leave_array[i] )
		{
			min = leave_array[i];
		}
	}
    if ( abs(max-min) < 400 )   // 手腕脱离判断，最大最小波幅差值<400.
	{                // 这是一个经验值，根据具体情况修改.
        if ( abs(max-min) < 40 )
        {
            is_leave_true = 1;
        }else{
            is_leave_true = 0;
        }
		return true;
	}else
	{
        is_leave_true = 0;
		return false;
	}
}

int leve_flag_ = 0;
char is_leave_flag = 0;

float hr_get_(void)
{
    return 0;
}

float hr_array[5] = {75.0f,75.0f,75.0f,75.0f,75.0f};
float hr_real;
const unsigned char Hr_Rand[] = {2,5,1,4,6,8,7,4,9,3};

short hr_rand_start[] = {70,85,75,83,74,72,81,79};
int hr_i = 0;
	
float hr_get(void)
{
	return ((hr_array[0]+hr_array[1]+hr_array[2]+hr_array[3]+hr_array[4])/5.0f);
}

void hr_reset(float hr)
{
	for ( int i = 0; i < SIZEOF(hr_array); i++ )
	{
		hr_array[i] = hr;
	}
}

void do_hr(void)
{
}

unsigned int leave_time,leaveFlag = 0;
unsigned char hr_reset_flag = 0;
unsigned char upload_flag = 0;

unsigned long taskHr( unsigned long task_id, unsigned long events )
{ 
	if ( events & TASK_HR_STOP_EVT )
	{
		hr_clock_rtc_stop();
		hr_pwr_enable(false);
		HR_ADC_PIN_OUTPUT_LOW();
		nrf_saadc_disable();
		sleepEnable = 1;
		
		/* 自动测量模式，马达静默 */
		if ( hr.auto_flag != 1 )
		{
			osal_set_event ( taskMotorTaskId, TASK_MOTOR_VIBORATE_EVT );
		}
		
		hr.auto_flag = 0;			
		
		hr.stop = 1; to_close = 0;
		leave_time = 0; leaveFlag = 0;
		
		osal_start_timerEx( taskUploadTaskId, TASK_UPLOAD_HR_DONE_EVT, 4000);
				
		return ( events ^ TASK_HR_STOP_EVT );
	}
	
	if ( events & TASK_HR_START_EVT )
	{
		hr_clock_rtc_start();
		hr_pwr_enable(true);
		sleepEnable = 0;
		
		hr_reset_flag = 1;
		
		hr.stop = 0;
		dect_mTicks = hr_mticks;
		hr_showFlag = 1;
		hr.fg = 0;
		upload_flag = 1;
        is_leave_true = 0;
        is_leave_flag = 0;        
		
		hr.stat = HR_DECT_STAT_DECTING;
		
		/* 自动测量模式，马达静默 */
		if ( hr.auto_flag != 1 )
		{
			osal_set_event ( taskMotorTaskId, TASK_MOTOR_VIBORATE_EVT );
		}
		// 8秒稳定后再进行检测,稳定时间由硬件决定.
		
		leaveEnable = false;
		osal_start_timerEx( task_id, TASK_HR_LEAVE_EVT, 4000); 
		
		osal_start_timerEx( task_id, TASK_HR_NORMAL_EVT, 100 );

		return ( events ^ TASK_HR_START_EVT );
	}
	
	if ( events & TASK_HR_LEAVE_EVT )
	{
		/**
		 * 运放模块稳定后2.1秒在闪动心率图标,避免与2秒脱腕检测时间冲突.
		 */
		leaveEnable = true; /* 使能脱腕检测 */
		
		dect_mTicks = hr_mticks;
		
		return ( events ^ TASK_HR_LEAVE_EVT );
	}
	
	if ( events & TASK_HR_NORMAL_EVT )
	{
		if ( hr.stat == HR_DECT_STAT_DECTING )
		{
			dect_mTicks = hr_mticks;
			
			hr.stat = HR_DECT_STAT_NORMAL; /* 进入正常检测模式, 允许图标闪动 */			
		}
		
		return ( events ^ TASK_HR_NORMAL_EVT );
	}
				
	if ( events & TASK_HR_SAMPLE_EVT )
	{
		if ( config.hrWaveUploadEnable == 1 )
		{
			Debug_ble_send_hex(current_hr_adc);
		}
		
		leave_array_put( current_hr_adc );
		
		#if 1
		if ( current_touch_adc < hr.touch_level )
		{
			// 2s Detecting time
			leave_time++;
			if ( leave_time > 50 )
			{
				leaveFlag = 1;
			}
		}else{
			leave_time = 0;
		}
		#endif
		
		#if 0
        if ( (leaveEnable == true && is_leave() == true) || 
            ( (leaveEnable == true) && (leaveFlag == 1) )  )
		{	
			if ( hr.stat == HR_DECT_STAT_DONE )
			{
				hr.stat = HR_DECT_STAT_STOP;
			}else{
				hr.stat = HR_DECT_STAT_ERROR;
			}
			
			hr.leave_flag = 1;
				
			osal_set_event( task_id, TASK_HR_STOP_EVT ); 
			
			return ( events ^ TASK_HR_SAMPLE_EVT );
		}
        #else
        
        if ( (leaveEnable == true) && (is_leave() == true) && is_leave_flag == 0 )
        {
            is_leave_flag = 1;
        }
        
        if ( ( (leaveEnable == true) && (leaveFlag == 1)) || ( (leaveEnable==true) && (is_leave_true==1)) )       
		{        
			/**
 			 * 已经脱腕, 停止测量, 状态更改为 HR_DECT_STAT_ERROR
			 * 界面显示 "X"
			 */
			// if ( hr.stat == HR_DECT_STAT_DONE )
			// {
				hr.stat = HR_DECT_STAT_NONE;
			// }else{
			//	hr.stat = HR_DECT_STAT_ERROR;
			// }
			osal_set_event( task_id, TASK_HR_STOP_EVT );

			hr.leave_flag = 1;			
			
			return ( events ^ TASK_HR_SAMPLE_EVT );
		}        
		#endif
			
		// if ( (hr_mticks - dect_mTicks) > 60000 )/* Note FFT 20480 Point */
		// 时间缩短为20秒(5x4s),刚好将滤波数组填充满; 2017.11.16
		// 增加2s 2017.11.23
		if ( (hr_mticks - dect_mTicks) > 38000 )/* Note FFT 20480 Point */
		{
			/**
			 * FFT采样频率为50Hz, 采样点数为1024点,
			 * 共需要1024*1/50s=20.48s的时间.
			 * 为使得尽快出心率,可以采集少于20.48s.
			 */    
			hr.stat = HR_DECT_STAT_STOP;
			osal_set_event( task_id, TASK_HR_STOP_EVT );

			/** 
			 * The automatic measurement mode stores only one data
			 */
			if ( hr.auto_flag == 1 )
			{
                /**
                 * 2018.05.09
                 * 确定一下,自动测量心率模式且处于睡眠中,心率不要超100
                 */ 
                
				// osal_set_event( task_id, TASK_HR_DECT_DONE_HR_EVT );
			}
            
            osal_set_event ( task_id, TASK_HR_DECT_DONE_HR_UPLOAD_EVT );
            osal_set_event( task_id, TASK_HR_DECT_DONE_HR_EVT );			
			
			return ( events ^ TASK_HR_SAMPLE_EVT );
		} 
		
		if ( ( (hr_mticks - dect_mTicks) > need_mTicks ) && (hr.fg == 0) )
		{
			/**
			 * 开始进行FFT运算 (每隔4秒)
			 * 第一个心率出来后,用该值填充hr_array数组.
			 * 如果心率值与上一次不等,则保存并上传.
			 * 血压和血氧上传规则与心率保持一致.
			 */
			hr.fg = 1;  hr.stat = HR_DECT_STAT_DONE;
											
			float temp = (int)hr_get();
			
			upload_flag++;
			
			if ( hr_reset_flag == 1 )
			{
				hr_reset(hr_real);
				temp = hr_real;
				hr_reset_flag = 0;
			}			
			
			if ( ((int)hr.hr != (int)temp ) || !(upload_flag % 2) )
			{
				do_hr();
				
				osal_set_event ( task_id, TASK_HR_DECT_DONE_HR_UPLOAD_EVT );
			}
			
			/** 
			 * The automatic measurement mode stores only one data
			 */			
			if ( ( ( (int)hr.hr != (int)temp ) || !(upload_flag % 4) )
					&& hr.auto_flag != 1 )
			{
				osal_set_event( task_id, TASK_HR_DECT_DONE_HR_EVT );
			}
			
			hr.hr = temp;
			osal_start_timerEx( task_id, TASK_HR_FFT_ENABLE_EVT, 3650);
		}		
			
		return ( events ^ TASK_HR_SAMPLE_EVT );
	}
	
	if ( events & TASK_HR_FFT_ENABLE_EVT )
	{
	    hr.fg = 0;
		
		return ( events ^ TASK_HR_FFT_ENABLE_EVT );
	}		
				
	if ( events & TASK_HR_DECT_DONE_HR_EVT )
	{
		/** 保存当前心率血压血氧值 */
		
		osal_set_event( taskStoreTaskId, TASK_STORE_SAVE_HR_EVT ); 
		osal_set_event( taskStoreTaskId, TASK_STORE_SAVE_SPO2_EVT);
		osal_set_event( taskStoreTaskId, TASK_STORE_SAVE_B_PRESSURE_EVT);		

		return ( events ^ TASK_HR_DECT_DONE_HR_EVT );
	}
	
	if ( events & TASK_HR_DECT_DONE_HR_UPLOAD_EVT )
	{
		/** 上传当前心率血压血氧值 */
		
		osal_set_event( taskUploadTaskId, TASK_UPLOAD_iBand_HR_DECT_EVT );
		osal_start_timerEx( taskUploadTaskId, TASK_UPLOAD_iBand_B_PRUESSURE_EVT, 75 );		
		osal_start_timerEx( taskUploadTaskId, TASK_UPLOAD_iBand_SPO2_EVT, 150 );
		
		return ( events ^ TASK_HR_DECT_DONE_HR_UPLOAD_EVT );
	}
	
#if (OTA_MODE_ENABLE==0)	
	
	if ( events & TASK_HR_SENSOR_OK_EVT )
	{		
		return ( events ^ TASK_HR_SENSOR_OK_EVT );
	}	
	
	if ( events & TASK_HR_SENSOR_COMFIR_EVTT )
	{
		hr_ok = 1;		
		return ( events ^ TASK_HR_SENSOR_COMFIR_EVTT );
	}
	
	if ( events & TASK_HR_PW_ENABLE_EVT )
	{
		if ( config.test_mdoe == 1 )
		{
			nrf_gpio_pin_pull_t config_1 = NRF_GPIO_PIN_PULLDOWN;
			nrf_gpio_cfg_input(HR_ADC_PIN, config_1);
			
			nrf_gpio_pin_pull_t config_2 = NRF_GPIO_PIN_PULLDOWN;
			nrf_gpio_cfg_input(HR_TOUCH_LEVEL_PIN, config_2);			
			hr_pwr_enable(true);
			
			osal_start_timerEx( task_id, TASK_HR_SENSOR_OK_EVT, 2300);
		}
		
		return ( events ^ TASK_HR_PW_ENABLE_EVT );
	}
#endif
	
	if ( events & TASK_HR_DECT_AUTO_EVT )
	{
		if ( (hr.auto_measure_enable == 0) ||
			hr.auto_measure_interval == 0 )
		{
			return ( events ^ TASK_HR_DECT_AUTO_EVT );
		}
			
		// osal_start_timerEx( task_id, TASK_HR_DECT_AUTO_EVT, hr.auto_measure_interval*60*1000);
		
		hr.start_hour = TimerHH();
		hr.start_minute = TimerMI();
		hr.start_seconds = TimerSE();
		
		// 计算终止时间
		hr.end_seconds = hr.start_seconds;
		hr.end_minute = hr.start_minute + hr.auto_measure_interval;
		if( hr.end_minute > 59 )
		{
			uint8_t over_hour = hr.end_minute/60;	// 如果分钟超出一个小时
			hr.end_hour = (hr.start_hour + over_hour);
			if( hr.end_hour > 23 )
			{
				hr.end_hour %= 24;
			}
			hr.end_minute %= 60;
		}else{
			hr.end_hour = hr.start_hour;
		}        
        
		if ( (hr.stop == 1) && (config.shutdown == 0) )
		{
			need_mTicks = 8000;
			osal_set_event( task_id, TASK_HR_START_EVT );
			
			hr.auto_flag = 1;
		}

		return ( events ^ TASK_HR_DECT_AUTO_EVT );
	}
	
	return 0;
}

void hr_pwr_enable( bool enable)
{
	if ( enable == true)
	{ 
		nrf_gpio_pin_clear(HR_PWR_PIN); 			
	}else{ 
		nrf_gpio_pin_set(HR_PWR_PIN); 			
	}
}

void taskHrInit( unsigned long task_id )
{
	taskHrTaskId = task_id;
	
	nrf_gpio_cfg(
            HR_PWR_PIN,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_pin_set(HR_PWR_PIN);	
	
	hr_clock_rtc_init();	
		
	HR_ADC_PIN_OUTPUT_LOW();
}
