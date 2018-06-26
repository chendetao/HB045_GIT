/*******************************************************************************
* Copytight 2017 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : Task_Step.c                                                       *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription :                                                                 *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017-3.30  1.0        build this moudle                   *
*                                                                              *
*                                                                              *
* Created : 2017-3-30                                                          *
* Last modified : 2017.3.30                                                    *
*******************************************************************************/
#include <stdlib.h>
#include <time.h>
#include "OSAL.h"
#include "OSAL_Task.h"
#include "OSAL_Timer.h"

#include "Task_Step.h"
#include "Task_Upload.h"
#include "Task_Battery.h"
#include "Task_Motor.h"
#include "Task_Flash.h"
#include "Task_GUI.h"
#include "Task_Hr.h"

#include "step_process.h"
#include "bma253.h"
#include "log.h"

#include "btprotocol.h"
#include "ble_wechat_service.h"
#include "UserSetting.h"
#include "ClockTick.h"
#include "UI_win_notify.h"
#include "Window.h"
#include "HeartComp.h"
#include "MsgQueue.h"

#include "UI_win_running.h"
#include "sleep_algorithm.h"
#include "UI_win_sport.h"

PedometerInfo_Typedef pedometer,pedometer_last;
unsigned long taskStepTaskId;
unsigned long action_cnt = 0;

struct bma2x2_accel_data_fifo accel_fifo[FIFO_DEPTH];

extern unsigned long taskStepInterrutCount;
extern unsigned long taskStepInterrutMonotorCnt;
unsigned long sendIndex = 0;
unsigned long sendCount = 0;

void load_pedometer(unsigned long last_pedometer)
{
	accelDevice.allCounter = accelDevice.oCounter = last_pedometer;
	
	for ( int i = 0; i < 3; i++ )
	{
		accelDevice.iCounter[i] = last_pedometer;
		accelDevice.Counter[i]  = last_pedometer;
		accelDevice.MaxThreshold[i]   = last_pedometer;
		accelDevice.MinThreshold[i]   = last_pedometer;
		accelDevice.MidleThreshold[i] = last_pedometer;
	}
	
	unit_update();
}

void unit_update(void)
{
    float t_distance, t_calorie;
    	
	int length;
	if ( config.height >= 148 )
	{
		length = 70 - (170 - (config.height ? config.height : 170)); 
	}else if ( config.height >= 100 )
	{
		length = 55;
	} else
	{
		length = 40;
	}
    
    t_distance = length * accelDevice.oCounter / 100.0;
	
    t_calorie  = (t_distance/(float)100.0f) * (float)0.0766666666667f*\
		(config.weight ? config.weight : 60);    
	
    pedometer.goal_per = (pedometer.goal ? (unsigned int)\
		(accelDevice.oCounter *100.0 / pedometer.goal + 0.5) : 0); 
                 
    pedometer.distance = (unsigned int)(t_distance); // 单位 : 米
    pedometer.calorie = (unsigned int)(t_calorie);   // 单位 : 卡
    pedometer.counter = accelDevice.oCounter; 

	m_pedometer_profile_val[0] = 0x1;
	m_pedometer_profile_val[1] = (pedometer.counter>>0) & 0xFF;
	m_pedometer_profile_val[2] = (pedometer.counter>>8) & 0xFF;
	m_pedometer_profile_val[3] = (pedometer.counter>>16) & 0xFF;	
}

#define UPLOAD_FLAG_ENABLE 0x00000001
unsigned long pedometerFlag = UPLOAD_FLAG_ENABLE;

int sum_x = 0, sum_y = 0, sum_z = 0;

volatile int callCnt = 0;
unsigned long d4time = 0;

static unsigned char local_tx_buf[20];

unsigned long taskStep( unsigned long task_id, unsigned long events )
{
	static unsigned long stepStatusMachine = 0, stepInterruptTime = 0;
	
	/***************************************************************************
     *	                                                                       *
	 * 读取GSensor数据                                                         *
	 *                                                                         *
	 **************************************************************************/
	if ( events  &  STEP_TASK_DECT_EVT )
	{ 
		const char *pbuf;
		
		/** Read the 3accel data from fifo buffer */
		
		BMA2X2_READ_FIFO_BUFFER(accel_fifo, 6 * FIFO_DEPTH);
		
		/** 
		 * Less than 8*50=400ms indicate a error ocurr, we
		 * need to re-configure the gsensor.
		 */
		d4time = osal_systemClock - stepInterruptTime ;
		
		stepInterruptTime = osal_systemClock;
		
        if ( (d4time < (8-1)) || (d4time > 10) ) /* 8*system tick(50ms) = 400ms */
		{
			osal_set_event( task_id, TASK_STEP_BMA_RE_INIT_EVT );
			
			return ( events ^ STEP_TASK_DECT_EVT );
		}			
		
		for ( unsigned int i = 0; i < FIFO_DEPTH; i++ )
		{
			pbuf = (char*)&accel_fifo[i];
			
            accel_fifo[i].x = (s16)((((s32)((s8)pbuf[1]))\
              << C_BMA2x2_EIGHT_U8X) | (pbuf[0] & BMA2x2_10_BIT_SHIFT));
            accel_fifo[i].x = accel_fifo[i].x >> C_BMA2x2_SIX_U8X;
                        
            accel_fifo[i].y = (s16)((((s32)((s8)pbuf[3]))\
              << C_BMA2x2_EIGHT_U8X) | (pbuf[2] & BMA2x2_10_BIT_SHIFT));
            accel_fifo[i].y = accel_fifo[i].y >> C_BMA2x2_SIX_U8X;
                        
            accel_fifo[i].z = (s16)((((s32)((s8)pbuf[5]))\
              << C_BMA2x2_EIGHT_U8X) | (pbuf[4] & BMA2x2_10_BIT_SHIFT));
            accel_fifo[i].z = accel_fifo[i].z >> C_BMA2x2_SIX_U8X;
		}
		
		{
			AccelHandle();
			osal_set_event( task_id, TASK_STEP_UPLOAD_PEDOMETER_EVT );
		}
		
		if ( (AutoSleep.out_sleepProcess == 0x1) || (AutoSleep.out_sleepEnable == 0x1))
        {
            osal_start_timerEx( task_id, TASK_STEP_SLEEP_ACTION_EVT, 10);
        }
		
		if ( config.control & BSTOTW_FLAG_ENABLE ) 
		{
			/* If configuration is Enable we do that */
			
			osal_set_event( task_id, TASK_STEP_BSTOTW_EVT );
        }
		
		if ( config.gsensor_debug == 1 )
		{
			sendIndex = 0;			
			
			if ( config.gsensor_fast_enable == 1 )
			{
				osal_set_event ( task_id, TASK_STEP_UPLOAD_FAST_WAVE_EVT );
			}else{
				osal_set_event ( task_id, STEP_TASK_UPLOAD_WAVE_EVT );
			}			
		}
		
		osal_start_timerEx( task_id, TASK_STEP_BMA_RE_INIT_EVT, 30000 ); 
		
		return ( events ^ STEP_TASK_DECT_EVT );
	}
	
	/***************************************************************************
     *	                                                                       *
	 * 根据3轴状态进行翻腕检测                                                 *
	 *                                                                         *
	 **************************************************************************/

	if ( events & TASK_STEP_BSTOTW_ENABLE_EVT )
	{
		config.enable  = 1;
		
		return ( events ^ TASK_STEP_BSTOTW_ENABLE_EVT );
	}
	
	if ( events & TASK_STEP_BSTOTW_EVT )		
	{ 
		config.control &= ~BSTOTW_FLAG_TEMP_ENABLE;
			
		for ( int i = 0; i < (16-5) ; i++ )
		{ 
			for( int j = 0; j < 5; j++)
			{
				sum_x += accel_fifo[i+j].x;
				sum_z += accel_fifo[i+j].z;
				sum_y += accel_fifo[i+j].y;
			}
			
			sum_z /= 5, sum_y /= 5;  sum_x /= 5;
			
			if (
				((sum_x > (-180)) && (sum_x < (130))) 
				&& ( sum_z < -220 ) 
			  && ((sum_y > (-180)) && (sum_y < (180))) )
			{
				config.control |= BSTOTW_FLAG_TEMP_ENABLE;
				break;
			}
			
			if ( (sum_y > 120) || (sum_y<-200) || (sum_x < -180) )  /* Turn over the wrist */
			{
				if ( !(config.control & BSTOTW_FLAG_TURN_OVER_ENABLE) )
				{
					config.control |= BSTOTW_FLAG_TURN_OVER_ENABLE;
				}
			}
			
			#if 0
			if (  sum_y < 30 && sum_y > -150 && sum_z > 200 )
			{
				if ( config.hflag == 0 && win_current == 11 
					&& mq.type == NOTIFY_TYPE_CALL)
				{
					config.hflag = 1;      /* 执行长按挂断(手环振动反馈) */
					
					osal_set_event( taskGUITaskId,TASK_GUI_PRESS_EVT );
					osal_set_event( taskMotorTaskId, TASK_MOTOR_BLE_CONNECT_EVT );
					
					/* 2秒后再次允许本操作 */
					osal_start_timerEx( task_id, TASK_STEP_HFLAG_CLEAR_EVT, 2000);
				}
			}
			#endif
		}
           
		if ( config.control & BSTOTW_FLAG_TEMP_ENABLE )
		{
			if ( config.lcdEnable == 0 && (config.enable == 1)
			//	&& ( config.control & BSTOTW_FLAG_CC_ENABLE )
				&& (config.shutdown == 0)  
			 && ( config.control & BSTOTW_FLAG_TURN_OVER_ENABLE) )
			{
				// Not in sleep mode and notify task and sport in mode
				
				if ( (win_current == WIN_ID_NOTIFY) || (sport_stat == 1)
                    || (win_current == WIN_ID_TAKE_PHOTO ) )
				{
					ui_window_manager( WINDOW_TOUCH, 0, 0);
				}else{
					win_current = 0;
                    select_enable = 0;
					ui_window_manager( WINDOW_CREATE, 0, 0);                
				}
				
				/* 避免屏幕点亮之后立即被关闭的情况,延时一定时间再清除标志位 
				* 修改该参数可以调整到达翻腕灭屏效果.
				*/
				// osal_start_timerEx( task_id, STEP_TASK_LCD_OFF_EVT, 4500);
				
				config.control &= ~BSTOTW_FLAG_TURN_OVER_ENABLE;
			}
		}else
		{
			if ( ( config.lcdEnable == 1 ) 
			&& ( !(config.control & BSTOTW_FLAG_CC_ENABLE)) )
			{
				// ui_window_manager( WINDOW_CLOSE, 0, 0);
			}
                
			config.control |= BSTOTW_FLAG_CC_ENABLE;
		}
		
		return ( events ^ TASK_STEP_BSTOTW_EVT );
	}
	
	if ( events & TASK_STEP_HFLAG_CLEAR_EVT )
	{
		config.hflag = 0;
		
		return ( events ^ TASK_STEP_HFLAG_CLEAR_EVT );
	}

	/**
     * 计算每个方向的加速度变化值，变化阀值大于10，认为产生动作
     */ 
    if ( events & TASK_STEP_SLEEP_ACTION_EVT  )
    {
		static int xo1 = 0,xo2 = 0,xo3 = 0;
		static int abs_x = 0,abs_y = 0, abs_z = 0;
        
		for ( int i = 0; i < 16; i += 2 )
		{
			abs_x = abs(accel_fifo[i].x - xo1);
			abs_y = abs(accel_fifo[i].y - xo2);
			abs_z = abs(accel_fifo[i].z - xo3);
					   
			if(( abs_x > 10 )|| ( abs_y > 10 )|| ( abs_z > 10 ) )
			{
				action_cnt++;
			}
					  
			xo1 = accel_fifo[i].x; xo2 = accel_fifo[i].y; xo3 = accel_fifo[i].z;
		}        
			  
		return ( events ^ TASK_STEP_SLEEP_ACTION_EVT  );
    }

    /***************************************************************************
    *                                                                          *
    *                   睡眠监测相关事件.                                      *
    *                                                                          *
    ***************************************************************************/
    
   	/* 每分钟监测一次 */
    if ( events & STEP_TASK_SLEEP_DETECT_TICK_EVT )
    {
        /* 1.脭莽脡脧9碌茫碌陆脰脨脦莽12碌茫拢卢虏禄陆酶脨脨脠毛脣炉录矛虏芒*/		
        
        /*认为此时放在桌面*/
        if ( power.charge_flage == 0x1 )
        {
           AutoSleep.out_sleepEnable = 0;
           AutoSleep.sleep_step      = 0x4;
        }
      
        sleep_algorithm(action_cnt, accelDevice.WaveCounter,
			accelDevice.WaveAmplitudeSum, &current_timer);
		
        accelDevice.WaveCounter      = 0x0;
        accelDevice.WaveAmplitudeSum = 0x0;
        action_cnt                   = 0x0;
		
        for ( int i = 0; i < AutoSleep.output_counter; i++ )
        {
            struct time_struct  sleep_stimer;
            struct time_struct  sleep_etimer;
            unsigned char buffer[16];
            
            memset(buffer, 0, 16);
            
            Cal_Time( AutoSleep.output_UTC_stime[i], &sleep_stimer);
            Cal_Time( AutoSleep.output_UTC_etime[i], &sleep_etimer);
            
            buffer[0] = ((sleep_stimer.year/10)   <<4)|(sleep_stimer.year%10);
            buffer[1] = ((sleep_stimer.month/10)  <<4)|(sleep_stimer.month%10);
            buffer[2] = ((sleep_stimer.day/10)    <<4)|(sleep_stimer.day%10);
            buffer[3] = ((sleep_stimer.hour/10)   <<4)|(sleep_stimer.hour%10);
            buffer[4] = ((sleep_stimer.minute/10) <<4)|(sleep_stimer.minute%10);
            
            buffer[5] = ((sleep_etimer.year/10)   <<4)|(sleep_etimer.year%10);
            buffer[6] = ((sleep_etimer.month/10)  <<4)|(sleep_etimer.month%10);
            buffer[7] = ((sleep_etimer.day/10)    <<4)|(sleep_etimer.day%10);
            buffer[8] = ((sleep_etimer.hour/10)   <<4)|(sleep_etimer.hour%10);
            buffer[9] = ((sleep_etimer.minute/10) <<4)|(sleep_etimer.minute%10);
            
            buffer[10] = AutoSleep.output_sleep_Counter[i]>>8;
            buffer[11] = AutoSleep.output_sleep_Counter[i] & 0xFF;
            
            buffer[12] = AutoSleep.output_sleep_type[i];
            
            // put into queue for save
			// flash_temp_queue_put(buffer, 16);
			flash_temp_queue_put(FM_ID_SLEEP, buffer, 16);
        }
        if ( AutoSleep.output_counter )
        {
            osal_set_event ( taskFlashTaskId, TASK_FLASH_QUEUE_PUT_EVT );
            osal_start_timerEx( task_id, STEP_TASK_NOTIFY_EVT, 8000);
        }
        
        AutoSleep.output_counter = 0x0;
        
        // add for debug
        osal_set_event ( task_id, STEP_TASK_NOTIFY_SLEEP_TEST_EVT );
        
        return ( events ^ STEP_TASK_SLEEP_DETECT_TICK_EVT );
    }
	    
    if ( events & STEP_TASK_NOTIFY_SLEEP_TEST_EVT )
    {
        /*虏芒脢脭脨颅脪茅
        0xF0   0x12  ss  0x55  cx   0x55  cnh  cnl  0x55  st  
        
        ss: 脣炉脙脽脳麓脤卢 00-03
        cx: 露炉脳梅1麓脦脢媒
        cnh: 露炉脳梅2麓脦脢媒赂脽掳脣脦禄
        cnl: 露炉脳梅2麓脦脢媒碌脥掳脣脦禄
        st:  脙驴脪禄路脰脰脫录脟虏陆
        */
        unsigned char ble_send_data[20];
		
        ble_send_data[0] = 0xF0;
        ble_send_data[1] = 0x12;
        
        ble_send_data[2] = AutoSleep.sleep_step;
        ble_send_data[3] = 0x55;
        
        ble_send_data[4] = AutoSleep.wave_cnt;
        ble_send_data[5] = 0x55;
        
        ble_send_data[6] = (AutoSleep.action_cnt)>>8;
        ble_send_data[7] = (AutoSleep.action_cnt);
        ble_send_data[8] = 0x55;
        
        ble_send_data[10] = AutoSleep.out_TotalSleepTime>>8;
        ble_send_data[11] = AutoSleep.out_TotalSleepTime & 0xFF;
        ble_send_data[12] = AutoSleep.out_TotalDeepTime>>8;
        ble_send_data[13] = AutoSleep.out_TotalDeepTime & 0xFF;
        ble_send_data[14] = AutoSleep.out_TotalLightTime>>8;
        ble_send_data[15] = AutoSleep.out_TotalLightTime & 0xFF;
        
        bt_protocol_tx(ble_send_data, 20);
        
        return ( events ^ STEP_TASK_NOTIFY_SLEEP_TEST_EVT );
    }
	
    if ( events & STEP_TASK_CLEAR_SLEEP_EVT )
    {
        AutoSleep.out_TotalSleepTime = 0x0;
        AutoSleep.out_TotalDeepTime  = 0x0;
        AutoSleep.out_TotalLightTime = 0x0;
		AutoSleep.out_TotalWakeTime  = 0x0;
        
        return ( events ^ STEP_TASK_CLEAR_SLEEP_EVT );
    }
	
	if ( events & STEP_TASK_NOTIFY_EVT)
	{
		if ( ble.isConnected == 0 )
		{
			return ( events ^ STEP_TASK_NOTIFY_EVT );
		}

		osal_set_event ( taskUploadTaskId, TASK_UPLOAD_SL_EVT );
		
		return ( events ^ STEP_TASK_NOTIFY_EVT );
	}	
	
	if ( events & STEP_TASK_TEST_EVT )
	{	
		return ( events ^ STEP_TASK_TEST_EVT );
	}
			
	if ( events & TASK_STEP_UPLOAD_PEDOMETER_EVT )
	{
		static unsigned long ooCounter = 0;
        
        int val = 9;
        if ( AutoSleep.out_sleepEnable == 1 )
        {
            val = 50;
        }
		
        if ( stepStatusMachine == 0 )
        {       
            /* 至少2秒内要走出一步，且连续十步有效  */
            
            if ( (accelDevice.allCounter - accelDevice.oCounter) > val )
            {
                stepStatusMachine = 1;              
                osal_stop_timerEx( task_id, TASK_STEP_CLEAR_EVT );              
                events &= ~(TASK_STEP_CLEAR_EVT);
                
                return ( events ^ TASK_STEP_UPLOAD_PEDOMETER_EVT );
            }

            if ( ooCounter != accelDevice.allCounter )
            {
                osal_start_timerEx( task_id, TASK_STEP_CLEAR_EVT, 1500 );              
                ooCounter = accelDevice.allCounter;
				
				AutoSleep.wave_cnt++;
            }                
            
            return ( events ^ TASK_STEP_UPLOAD_PEDOMETER_EVT );
        }
        
        if ( accelDevice.oCounter != accelDevice.allCounter )
        {   
            accelDevice.oCounter = accelDevice.allCounter;                      
            unit_update();
			
			// ADD for sleep
            
			AutoSleep.wave_cnt++;
			
            /* No valid pedometer dect in 2.2s set flag to zero */
            osal_start_timerEx( task_id, TASK_STEP_CLEAR_EVT, 2200 );
			
#if 0	    /* 取消延时上传(客户觉得体验不好) */
			
			/* Upload current pedometer */			
			if ( !(pedometerFlag & UPLOAD_FLAG_ENABLE) ) 
			{
				return ( events ^ TASK_STEP_UPLOAD_PEDOMETER_EVT );
			}
			/* Upload Intervel is 4s */
			
			pedometerFlag ^= UPLOAD_FLAG_ENABLE;
			osal_start_timerEx ( task_id, TASK_STEP_UPLOAD_ENABLE_EVT, 2000);
#endif						
			osal_set_event ( taskUploadTaskId, TASK_UPLOAD_PEDO_EVT );			
	    }		
		
		return ( events ^ TASK_STEP_UPLOAD_PEDOMETER_EVT );
	}
	
	/**
	 * New pedometer not large than 10 and no more step generate
	 * we clear the flag and counter set to previous vlaues. 
	 */
	if ( events & TASK_STEP_CLEAR_EVT )
	{
        accelDevice.allCounter  = accelDevice.oCounter;
		
		for ( int i = 0; i < 3; i++ )
		{
			accelDevice.Counter[i]  = accelDevice.oCounter;
			accelDevice.iCounter[i] = accelDevice.oCounter;
        }
		
        stepStatusMachine = 0;
		
		return ( events ^ TASK_STEP_CLEAR_EVT );
	}
	
	if ( events & TASK_STEP_UPLOAD_ENABLE_EVT )
	{
		/* Enable upload */
		
		pedometerFlag |= UPLOAD_FLAG_ENABLE;
		
		/** 
		 * upload last pedometer to keep display correspondence
		 * with app and device.
		 */
		osal_set_event ( taskUploadTaskId, TASK_UPLOAD_PEDO_EVT );
		
		return ( events ^ TASK_STEP_UPLOAD_ENABLE_EVT );
	}
	
	/***************************************************************************
	 *                                                                         *
     * 久坐提醒检测事件                                                        *
	 *                                                                         *
	 **************************************************************************/
	
	if ( events & TASK_STEP_SED_NOTI_EVT )
	{
        unsigned long temp, tt, flag = 0;
        
        if ( ( monitor.ss & 0x1) && (monitor.interval > 0) )
        {
			osal_set_event( task_id, TASK_STEP_SED_INIT_EVT );
        }        
        
        if ( (pedometer.counter - monitor.last_pedometer) > monitor.pedometer )
        {
            monitor.last_pedometer = pedometer.counter;
            
            return ( events ^ TASK_STEP_SED_NOTI_EVT );
        }
        
        monitor.last_pedometer = pedometer.counter;
        
        /* 条件 1，查看是否开启久坐提醒功能 */
        if ( !(monitor.ss & 0x1) )
        {
            return ( events ^ TASK_STEP_SED_NOTI_EVT );
        }
        
        temp = (unsigned long)(TimerHH() * 60) + TimerMI();
        
        /* 条件 2，满足1时，查看是否处于提醒时段 */
        if ( monitor.end > monitor.start )
        {
            if ( temp > monitor.end || temp < monitor.start )
            {
                return ( events ^ TASK_STEP_SED_NOTI_EVT );
            }
        }else
        {
            tt = monitor.end + (unsigned long)(24*60) - monitor.start;
            
            if ( temp < monitor.start && temp > tt )
            {
                return ( events ^ TASK_STEP_SED_NOTI_EVT );
            }
        }

        /**
 		 *  条件 3，满足1、2时查看是否开启面打扰
         *  免打扰关闭，直接提醒
         *  免打扰开启，查看是否处于免提醒范围...
		 */
        if ( (monitor.ss >> 1) & 0x1 ) 
        { 	/* 免打扰开启 */
           
            if ( monitor.sedentary_end > monitor.sedentary_start )
            {
                if ( (temp > monitor.sedentary_start) 
					&& (temp < monitor.sedentary_end) )
                {
                    return ( events ^ TASK_STEP_SED_NOTI_EVT );
                }else
                {
                    flag = 1;
                }
            }
            
            if ( monitor.sedentary_end < monitor.sedentary_start )
            {
                tt = monitor.sedentary_end + (unsigned long)(24*60) 
					- monitor.sedentary_start;
                
                if ( (temp > monitor.sedentary_start) && (temp < tt ) )
                {
                    return ( events ^ TASK_STEP_SED_NOTI_EVT );
                }else
                {
                    flag = 1;
                }
            }            
        }else 
        {
			/** 免打扰关闭 */
            flag = 1;
        }
        
		/**
		 * If Sedentary reminder flag is set and
		 * system is not shutdown, we make a notification
		 */	
        if ( (flag == 1) && ( config.shutdown == 0) )
        {
            NOTICE_ADD(NOTIFY_ID_SPORT);
        }
       	
		return ( events ^ TASK_STEP_SED_NOTI_EVT );
	}
	
	/***************************************************************************
	 *                                                                         *
	 * Heart Rate Compensate Events                                            *
	 * After that we upload current heart rate value                           *
	 *                                                                         *
	 ***************************************************************************/
    if ( events & TASK_STEP_HR_COMPENT_EVT )
    {				
        heart_comp_pedometer( pedometer.counter );       
 
        return ( events ^ TASK_STEP_HR_COMPENT_EVT );
    }	

	/***************************************************************************
	 *                                                                         *
	 * clear all counter anytime is needed                                     *
	 *                                                                         *
	 ***************************************************************************/	

	if ( events & TASK_STEP_CLEAR_ALL_EVT )
	{
        accelDevice.allCounter = accelDevice.oCounter = 0;
		
		for ( int i = 0; i < 3; i++ )
		{
			accelDevice.Counter[i]  = 0;
			accelDevice.iCounter[i] = 0;
        }		
		
		pedometer.calorie  = 0;
		pedometer.counter  = 0;
		pedometer.distance = 0;
		
		return ( events ^ TASK_STEP_CLEAR_ALL_EVT );
	}

	/***************************************************************************
	 *                                                                         *
	 * To Iniatize the sedentary reminder                                      *
	 *                                                                         *
	 ***************************************************************************/

	if ( events & TASK_STEP_SED_INIT_EVT )
	{
		if ( monitor.interval == 0 )
		{                                            
			return ( events ^ TASK_STEP_SED_INIT_EVT );			
		}
		
		/* Reset counter */
		monitor.last_pedometer = pedometer.counter;
		
		osal_start_timerEx( task_id, TASK_STEP_SED_NOTI_EVT,\
		monitor.interval*60000 );
		
		return ( events ^ TASK_STEP_SED_INIT_EVT );
	}

	/***************************************************************************
	 *                                                                         *
	 * To Iniatize the gsensor bma253(or bma250e)                              *
	 *                                                                         *
	 ***************************************************************************/	
	if ( events & TASK_STEP_BMA_INIT_EVT )
	{
		if ( bma250_init() == 0 )
		{		
			osal_start_timerEx( task_id, STEP_TASK_INT_COMFIRM_EVT, 1600);
		}
		
		osal_start_timerEx( task_id, TASK_STEP_BMA_RE_INIT_EVT, 5000);

		return ( events ^ TASK_STEP_BMA_INIT_EVT );
	}

	/***************************************************************************
	 *                                                                         *
	 * To reiniatize the gsensor bma253(or bma250e) if it not interruption     *
	 * out of 10s                                                              *
	 *                                                                         *
	 ***************************************************************************/	
	if ( events & TASK_STEP_BMA_RE_INIT_EVT )
	{
		osal_start_timerEx( task_id, TASK_STEP_BMA_RE_INIT_EVT, 5000);
		
		if ( config.shutdown == 0 )
		{
			if ( bma250_init() == 0 )
			{
				gsensor_ok = 1;
			}
		}else{
			gsensor_ok = 2;			
		}
		
		return ( events ^ TASK_STEP_BMA_RE_INIT_EVT ); 
	}
	
	if ( events & STEP_TASK_INT_COMFIRM_EVT )
	{
		if ( (taskStepInterrutCount > 2) && (taskStepInterrutCount < 6) )
		{
			gsensor_ok = 1;
			
			if ( config.motor_test_enable == 1 ){
				osal_set_event( taskMotorTaskId, TASK_MOTOR_BMA_DONE_EVT );
			}
		}else{
			gsensor_ok = 2;
		}
		
		return ( events ^ STEP_TASK_INT_COMFIRM_EVT );
	}
	
	if ( events & TASK_STEP_MONITOR_COMFIRM_EVT )
	{
		taskStepInterrutMonotorCnt = taskStepInterrutCount;
		
		osal_start_timerEx( task_id, TASK_STEP_MONITOR_COMFIRM_EVT_1, 800);
		
		return ( events ^ TASK_STEP_MONITOR_COMFIRM_EVT );
	}
	
	if ( events & TASK_STEP_MONITOR_COMFIRM_EVT_1 )
	{
		memset( local_tx_buf, 0, sizeof(local_tx_buf));
		
		local_tx_buf[0] = 0x0F;
		local_tx_buf[1] = 0x13;
		
		if ( taskStepInterrutCount > taskStepInterrutMonotorCnt )
		{
            local_tx_buf[2] = 0x81;            
		} else {
			local_tx_buf[2] = 0x80;
		}  	

		bt_protocol_tx( local_tx_buf, sizeof(local_tx_buf) );
		
		return ( events ^ TASK_STEP_MONITOR_COMFIRM_EVT_1 );
	}
	
	/***************************************************************************
	 *                                                                         *
	 * To upload the gsensor bma253 data for debugging                         *
	 *                                                                         *
	 ***************************************************************************/	

    if ( events & TASK_STEP_UPLOAD_FAST_WAVE_EVT )  /** 专用测试接口400ms 16点 */
    {	
		local_tx_buf[0] = 0x14;
		local_tx_buf[1] = 0x03;
		local_tx_buf[2] = (sendCount >> 8 ) & 0xFF;
		local_tx_buf[3] = sendCount & 0xFF;
		
		if ( sendIndex < FIFO_DEPTH )
		{	
			osal_start_timerEx( task_id, TASK_STEP_UPLOAD_FAST_WAVE_EVT, 50 );
			
			local_tx_buf[2+2]  = accel_fifo[sendIndex+0].x>>8;
			local_tx_buf[3+2]  = accel_fifo[sendIndex+0].x & 0xFF;
			local_tx_buf[4+2]  = accel_fifo[sendIndex+0].y>>8;
			local_tx_buf[5+2]  = accel_fifo[sendIndex+0].y & 0xFF;
			local_tx_buf[6+2]  = accel_fifo[sendIndex+0].z>>8;
			local_tx_buf[7+2]  = accel_fifo[sendIndex+0].z & 0xFF;
			
			local_tx_buf[8+2]  = accel_fifo[sendIndex+1].x>>8;
			local_tx_buf[9+2]  = accel_fifo[sendIndex+1].x & 0xFF;
			local_tx_buf[10+2] = accel_fifo[sendIndex+1].y>>8;
			local_tx_buf[11+2] = accel_fifo[sendIndex+1].y & 0xFF;
			local_tx_buf[12+2] = accel_fifo[sendIndex+1].z>>8;
			local_tx_buf[13+2] = accel_fifo[sendIndex+1].z & 0xFF;		
			
			sendIndex += 2;
		}else{
			sendIndex = 0;
			
			return ( events ^ TASK_STEP_UPLOAD_FAST_WAVE_EVT );
		}

		sendCount++;
		
		bt_protocol_tx( local_tx_buf, sizeof(local_tx_buf) );
        
        return ( events ^ TASK_STEP_UPLOAD_FAST_WAVE_EVT );
    }    
	
	if ( events & STEP_TASK_UPLOAD_WAVE_EVT )   /** 通用测试接口400ms 1点 */
	{
		memset(local_tx_buf,0,sizeof(local_tx_buf));
		
		local_tx_buf[0]  = 0x14;
		local_tx_buf[1]  = 0x02;	

		local_tx_buf[2]  = accel_fifo[0].x>>8;
		local_tx_buf[3]  = accel_fifo[0].x & 0xFF;
		local_tx_buf[4]  = accel_fifo[0].y>>8;
		local_tx_buf[5]  = accel_fifo[0].y & 0xFF;
		local_tx_buf[6]  = accel_fifo[0].z>>8;
		local_tx_buf[7]  = accel_fifo[0].z & 0xFF;
        local_tx_buf[8]  = 0x55; // data valid flag(0x55)
		local_tx_buf[9]  = (sendIndex>>8) & 0xFF;
		local_tx_buf[10] = sendIndex & 0xFF;
//		local_tx_buf[11] = (current_touch_adc>>8)&0x7F;
//		local_tx_buf[12] = current_touch_adc & 0xFF;
		local_tx_buf[13] = hr.leave_flag;
		local_tx_buf[14] = hr.hr;
		
		sendIndex++;
		
		bt_protocol_tx( local_tx_buf, sizeof(local_tx_buf) );
		
		return ( events ^ STEP_TASK_UPLOAD_WAVE_EVT );
	}     
	
	return ( 0 );
}

void taskStepInit( unsigned long task_id )
{
	taskStepTaskId = task_id;
	gsensor_ok = 0;	
	
	heart_comp_init ( config.age, config.height, config.weight );
	
	osal_set_event(task_id, TASK_STEP_BMA_INIT_EVT );
}
