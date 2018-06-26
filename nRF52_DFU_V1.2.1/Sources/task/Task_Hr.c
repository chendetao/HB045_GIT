#include <stdlib.h>
#include "OSAL.h"
#include "OSAL_Timer.h"
#include "OSAL_Task.h"
#include "Window.h"
#include "ClockTick.h"

#include "Task_Hr.h"
#include "Task_Upload.h"
#include "Task_Motor.h" 
#include "Task_Store.h"
#include "UserSetting.h"
#include "HeartComp.h"

#include "Board.h"
#include "nrf_saadc.h"
#include "nrf_gpio.h"
#include "Debug.h"

#include "em70x8.h"
#include "nrf_delay.h"

void hr_pwr_enable( bool enable);

void hr_timer_disable(void)
{
	hr_clock_rtc_stop();
	hr_pwr_enable(false);
	nrf_saadc_disable();
	
	sleepEnable = 1;	
}

unsigned char taskHrTaskId;
int to_close;

bool leaveEnable = false;
unsigned long dect_mTicks = 0, need_mTicks = 0;

#include "em70x8.h"

void hr_callback(void)
{		
	EM7028_hrs_get_data();
}
 
unsigned short leave_time;
unsigned char leaveFlag = 0;
unsigned char upload_flag = 0;
unsigned char leave_detec_flag = 0;
const unsigned char sp_h[] = {98,98,98,98,99,98,99,98,98,98,98,98,97,98,97,97,98,99,98,98,97,97,98,99};
const unsigned char sp_l[] = {2,4,3,5,4,1,2,6,8,9,4,4,5,5,7,4,2,1,4,9};
unsigned char sp_idx = 0;
const unsigned char bp_h[] = {128,130,126,123,127,121,120,125,119,116,120,114,116,115,123,131,132,126,};
const unsigned char bp_l[] = {75, 76, 74, 73, 74, 72, 70, 72, 69, 68, 69, 70, 71, 70, 74, 80, 83, 76};
unsigned char bp_idx = 0;

unsigned char hr_filter[] = {75,75,75,75,75};

unsigned char get_hr( unsigned char hr )
{
    hr_filter[0] = hr_filter[1];
    hr_filter[1] = hr_filter[2];
    hr_filter[2] = hr_filter[3];
    hr_filter[3] = hr_filter[4];
    hr_filter[4] = hr;
    
    int sum = hr_filter[0] + hr_filter[1] + hr_filter[2]
        + hr_filter[3] + hr_filter[4];
    
    return sum/5;
}    
    
unsigned long taskHr( unsigned long task_id, unsigned long events )
{ 
	if ( events & TASK_HR_STOP_EVT )
	{
		hr_clock_rtc_stop();
        EM7028_enable(false);
		hr_pwr_enable(false);
		
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
		hr_pwr_enable(true);
		nrf_delay_us(47);
		   
		dect_mTicks = hr_mticks;		
		hr.stop = 0;
		upload_flag = 1;
        leave_detec_flag = 0;
        
        EM7028_hrs_init();
        hr_clock_rtc_start();
		
		hr.stat = HR_DECT_STAT_DECTING;
        
        sp_idx = TimerSE() % SIZEOF(sp_l);
        bp_idx = TimerSE() % SIZEOF(bp_h);
        hr.sp_h = sp_h[TimerHH()];
		
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
			osal_set_event( task_id, TASK_HR_DATA_PROCESSING_EVT );            
		}
		
		return ( events ^ TASK_HR_NORMAL_EVT );
	}
				
	if ( events & TASK_HR_DATA_PROCESSING_EVT )
	{
        if ( hr.stop == 1 )
        {
            return ( events ^ TASK_HR_DATA_PROCESSING_EVT );
        }
        
		if ( config.hrWaveUploadEnable == 1 )
		{
			Debug_ble_send_hex(data>>4);
		}

        #if 1 
        if ( ( data<10000) || (data>60000 )) 
        {
            leave_time++;
            if ( leave_time > 3 )
            {
                leaveFlag = 1;
            }
			dect_mTicks = hr_mticks;
        }else{
            leave_time = 0;
        }
        #endif
        
        if ( ((leaveEnable==true) && (leaveFlag == 1)) )
        {
            if ( hr.stat == HR_DECT_STAT_DONE )
            {
                hr.stat = HR_DECT_STAT_STOP;
            }
            else{
                hr.stat = HR_DECT_STAT_ERROR;
            }
            
            osal_set_event ( task_id, TASK_HR_STOP_EVT );
            
            return ( events ^ TASK_HR_DATA_PROCESSING_EVT );
        }        
			
		if ( (hr_mticks - dect_mTicks) > 40000 )/* Note FFT 20480 Point */
		{
			/**
			 * FFT采样频率为50Hz, 采样点数为1024点,
			 * 共需要1024*1/50s=20.48s的时间.
			 * 为使得尽快出心率,可以采集少于20.48s.
			 */    
			
			hr.stat = HR_DECT_STAT_STOP;
            osal_set_event ( task_id, TASK_HR_STOP_EVT );
			osal_set_event( task_id, TASK_HR_DECT_DONE_HR_EVT );
            osal_set_event ( task_id, TASK_HR_DECT_DONE_HR_UPLOAD_EVT );
			
			hr.leave_flag = 0;
			
			return ( events ^ TASK_HR_DATA_PROCESSING_EVT );
		} 

		if ( (hr_mticks - dect_mTicks) > need_mTicks )
		{
			/**
			 * 开始进行FFT运算 (每隔4秒)
			 * 第一个心率出来后,用该值填充hr_array数组.
			 * 如果心率值与上一次不等,则保存并上传.
			 * 血压和血氧上传规则与心率保持一致.
			 */
			hr.stat = HR_DECT_STAT_DONE;
			
			upload_flag++;
						
            int tmp = get_hr(bpm_tmp);
						
			if ( (int)hr.hr != tmp )
			{
                hr.hr = tmp;
            
                hr.sp_l = sp_l[sp_idx]; sp_idx++; sp_idx %= SIZEOF(sp_l);
                hr.hp   = bp_h[bp_idx];
                hr.lp   = bp_l[bp_idx]; bp_idx++; bp_idx %= SIZEOF(bp_h);
                
                osal_set_event ( task_id, TASK_HR_DECT_DONE_HR_UPLOAD_EVT );
            }
				
            if ( !(upload_flag % 2 ) )
            {
                osal_set_event ( task_id, TASK_HR_DECT_DONE_HR_UPLOAD_EVT );
            }
		}

        osal_start_timerEx ( task_id, TASK_HR_DATA_PROCESSING_EVT, 1500 );
			
		return ( events ^ TASK_HR_DATA_PROCESSING_EVT );
	}
					
	if ( events & TASK_HR_DECT_DONE_HR_EVT )
	{
		/** 保存当前心率血压血氧值 */
		
		if ( hr.auto_flag == 0 )
        {		
            osal_set_event( taskStoreTaskId, TASK_STORE_SAVE_HR_EVT ); 
            osal_set_event( taskStoreTaskId, TASK_STORE_SAVE_SPO2_EVT);		
            osal_set_event( taskStoreTaskId, TASK_STORE_SAVE_B_PRESSURE_EVT );			
		} else {
            osal_set_event( taskStoreTaskId, TASK_STORE_SAVE_HR_EVT );			
		}
	
		return ( events ^ TASK_HR_DECT_DONE_HR_EVT );
	}
	
	if ( events & TASK_HR_DECT_DONE_HR_UPLOAD_EVT )
	{
		/** 上传当前心率血压血氧值 */
		
		if ( hr.auto_flag == 0 )
		{
            osal_set_event( taskUploadTaskId, TASK_UPLOAD_iBand_HR_DECT_EVT );
            osal_start_timerEx( taskUploadTaskId, TASK_UPLOAD_iBand_SPO2_EVT,100 );
            osal_start_timerEx( taskUploadTaskId, TASK_UPLOAD_iBand_B_PRUESSURE_EVT,200 );
		}
		
		return ( events ^ TASK_HR_DECT_DONE_HR_UPLOAD_EVT );
	}
	
	if ( events & TASK_HR_DECT_AUTO_EVT )
	{
		if ( (hr.auto_measure_enable == 0) ||
			hr.auto_measure_interval == 0 )
		{
			return ( events ^ TASK_HR_DECT_AUTO_EVT );
		}
			
		osal_start_timerEx( task_id, TASK_HR_DECT_AUTO_EVT, hr.auto_measure_interval*60*1000);
		
		if ( (hr.stop == 1) && (config.shutdown == 0) )
		{
			need_mTicks = 8000;
			osal_set_event( task_id, TASK_HR_START_EVT );
			
			hr.auto_flag = 1;
		//	hr.measure = (0x1<<0)|(0x1<<2);
		}

		return ( events ^ TASK_HR_DECT_AUTO_EVT );
	}
    
    if ( events & TASK_HR_INIT_EVT )
    {
        // 电源引脚
        nrf_gpio_cfg_output(HR_PWR_PIN);
        nrf_gpio_pin_set(HR_PWR_PIN);
	
        // I2C接口初始化
		nrf_gpio_cfg_output(HRS_SDA_PIN);
		nrf_gpio_pin_clear(HRS_SDA_PIN);
		nrf_gpio_cfg_output(HRS_SCL_PIN);
		nrf_gpio_pin_clear(HRS_SCL_PIN);
        
        // 心率定时器初始化
        hr_clock_rtc_init();

		if ( hr.auto_measure_enable == 1 )
		{
			osal_set_event ( task_id, TASK_HR_DECT_AUTO_EVT );
		}        
        
        return ( events ^ TASK_HR_INIT_EVT );
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
 
		nrf_gpio_pin_clear(HRS_SDA_PIN); 
		nrf_gpio_pin_clear(HRS_SCL_PIN);        
	}
}

void taskHrInit( unsigned long task_id )
{
	taskHrTaskId = task_id;
	
	osal_start_timerEx( task_id, TASK_HR_INIT_EVT, 5000);
}
