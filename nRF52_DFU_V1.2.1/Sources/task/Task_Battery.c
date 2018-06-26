#include "string.h"
#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_Battery.h"

#include "btprotocol.h"
#include "ClockTick.h"
#include "SSADC.h"
#include "Board.h"
#include "nrf_gpio.h"
#include "UserSetting.h"
#include "UI_win_sport.h"
#include "Window.h"

Power_Typdef power = 
{
	.battery_level 		= 0,
	.battery_low   		= 0,
	.charge_flage  		= 0,
	.ble_signal    		= 0,
	.compelete 	   		= 0,
	.time_after_last 	= 0,
};

short battery_filter_arr[5];

static short adc_filter( unsigned short current )
{
    long sum = 0;
    
    for ( int i = 0; i < SIZEOF(battery_filter_arr)-1; i++ ){
        battery_filter_arr[i] = battery_filter_arr[i+1];
    }
    
    battery_filter_arr[SIZEOF(battery_filter_arr)-1] = current;
    
    for ( int i = 0; i < SIZEOF(battery_filter_arr); i++ ){
        sum += battery_filter_arr[i];
    }
    
    return (short)( (float)sum / SIZEOF(battery_filter_arr) );
}

short battery_level_arr[31];

static float battery_get_level(unsigned short sample_adc)
{
    /* 4.18 */
    short max_value = 0x0316;   /* 4.18 V */
    short min_value = 0x024E;   /* 3.00+ V */
    
	sample_adc = (sample_adc < min_value)?(min_value):(sample_adc);    
	sample_adc = (sample_adc > max_value)?(max_value):(sample_adc);
    
    sample_adc -= min_value;
    
    float battery_level = (unsigned char)(sample_adc * (100.0/(max_value - min_value)));
	unsigned char level;
	if ( battery_level > 6 ) {
		level = (battery_level - 6) * 100.0f/(100-6);
	}else{
		level = 0;
	}
	
    long sum = 0;
    
    for ( int i = 0; i < SIZEOF(battery_level_arr)-1; i++ ){
        battery_level_arr[i] = battery_level_arr[i+1];
    }
    
    battery_level_arr[SIZEOF(battery_level_arr)-1] = level;
    
    for ( int i = 0; i < SIZEOF(battery_level_arr); i++ ){
        sum += battery_level_arr[i];
    }	
	
    return (power.charge_flage==1)?(float)sum / SIZEOF(battery_level_arr):level;	
}

unsigned long taskBatteryTaskId;
short current_adc;

#define POWER()  nrf_gpio_pin_read(BAT_CHARGE_PIN)
#define CHARE()  nrf_gpio_pin_read(BAT_STAT_DET_PIN)
#define LED_ALL_OFF()
#define LED_GREEN_ON()
#define LED_RED_ON()

unsigned char last_battery_level;
unsigned char battery_flag = 0;

unsigned long taskBattery( unsigned long task_id, unsigned long events )
{
	if ( events & TASK_BATTERY_DECT_EVT )
	{
		short * p_adc_value/*, current_adc */;
				
		if ( win_current == WIN_ID_HR )
		{
           return ( events ^ TASK_BATTERY_DECT_EVT );
		}			
		
		saadc_init();

		p_adc_value = saadc_buffer_convert();
		if ( (short)(p_adc_value[0]) < 0 ){
			p_adc_value[0] = 0;
		}
		if ( (short)(p_adc_value[0]) > 4095 )
		{
			p_adc_value[0] = 4095;
		}
		
		current_adc = adc_filter(p_adc_value[0]);
        
		/**
		 * 电量: 90秒后,非充电状态下，强制电量只允许变少,不允许变多，
		 * 看TMD还没有没有话说,艹 
		 */
		if ( (systemTick < 90) || (power.charge_flage == 1) )
		{
			power.battery_level = (unsigned char)battery_get_level( current_adc );
			last_battery_level = power.battery_level; 
		} else{
			unsigned char level = (unsigned char)battery_get_level( current_adc );
			if ( level <= last_battery_level )
			{
				last_battery_level = power.battery_level = level;
			}
		}
		
        #if 0
		osal_set_event ( task_id, TASK_BATTERY_PROTECT_EVT ); 
			#else
        #warning "............."
        #endif
		return ( events ^ TASK_BATTERY_DECT_EVT );
	}
	
	if ( events & TASK_BATTERY_PROTECT_EVT )
	{
		if ( (power.battery_level < 1) && (power.charge_flage == 0)
				&& (config.shutdown == 0) && (systemTick > 10) )
		{
			win_current = WIN_ID_SHUTDOWN;
			ui_window_manager( WINDOW_CREATE, 0, 0);
			ui_window_manager( WINDOW_PRESS, 0, 0);
		}
		
		return ( events ^ TASK_BATTERY_PROTECT_EVT );
	}
	
	if ( events & TASK_BATTERY_FEED_ARRAY_EVT )
	{
		static int feed_cnt = 0;
		
		short * p_adc_value/*, current_adc */;		
		
		saadc_init();		
		p_adc_value = saadc_buffer_convert();
		
		if ( (short)(p_adc_value[0]) < 0 ){
			p_adc_value[0] = 0;
		}
		if ( (short)(p_adc_value[0]) > 4095 )
		{
			p_adc_value[0] = 4095;
		}			
		
		current_adc = adc_filter(p_adc_value[0]);
		
		power.battery_level = (unsigned char)battery_get_level( current_adc );
		last_battery_level = power.battery_level;

		if ( ++feed_cnt < SIZEOF(battery_level_arr) * 4 )
		{
			osal_start_timerEx( task_id, TASK_BATTERY_FEED_ARRAY_EVT, 50);
		}
		
		return ( events ^ TASK_BATTERY_FEED_ARRAY_EVT );
	}
	
	if ( events & TASK_BATTERY_CHARGE_EVT )
	{
		battery_flag = 0;
		osal_start_timerEx( task_id, TASK_BATTERY_RESET_FLAG_EVT, 45000 );
		
		osal_set_event( task_id, TASK_BATTERY_FEED_ARRAY_EVT );
		
		/**
		 * 电源插入，充电引脚由低至高，电量100% 时,亮绿灯，表明充满.
  	     */
		if ( (POWER() == 1) && (CHARE() == 1) )
		{
			LED_ALL_OFF(); LED_GREEN_ON();
			   
			power.compelete    = 1;
			power.charge_flage = 0;
			
			power.time_after_last = 0;
			   
			return ( events ^ TASK_BATTERY_CHARGE_EVT );  
		}
		  
		if ( POWER() == 0 )
		{
			power.charge_flage = 0;                                                                          
		}
		                                                                       
		if ( (POWER() == 1) && (CHARE() == 0) )
		{
			power.compelete = 0;
		}
		  
		/**
         * 电源插入，充电状态脚由高至低：正在充电
	     */
		if ( (POWER() == 1) && (CHARE() == 0) ) 
  	      /* && (power.compelete == 1) ) || (power.compelete == 0)*/
		{
			LED_ALL_OFF(); LED_RED_ON();
			  
			if ( power.charge_flage == 0 )
			{
				if ( config.shutdown != 0 ) /* 充电自动开机 */
				{
					ui_window_manager( WINDOW_PRESS, 0, 0); 
				}else{
							   
 					if ( (win_current == WIN_ID_NOTIFY) || (sport_stat == 1) ) 
					{
						ui_window_manager( WINDOW_TOUCH, 0, 0);
					}else
					{
						win_current = 0;
						ui_window_manager( WINDOW_CREATE, 0, 0);
					}
				}
			}
		     
		   if ( (power.battery_level < 100) || (battery_flag == 0) )
		   {				  
			  power.charge_flage = 1;
		   }else{
			  power.charge_flage = 0; 
		   }
		  
		   return ( events ^ TASK_BATTERY_CHARGE_EVT );  
		}
		 
		/**
	     * 正常耗电状态(不充电)
	     */
	    LED_ALL_OFF(); power.charge_flage = 0;     
		
		return ( events ^ TASK_BATTERY_CHARGE_EVT );
	}
	
	if ( events & TASK_BATTERY_RESET_FLAG_EVT )
	{
		if ( power.battery_level >= 100 )
		{
			power.charge_flage = 0;
			
			osal_set_event ( task_id, TASK_BATTERY_QUERY_EVT );
		}
		
		battery_flag = 1;
		
		return ( events ^ TASK_BATTERY_RESET_FLAG_EVT );
	}
	
	if ( events & TASK_BATTERY_QUERY_EVT )
	{
		// Upload current battery level
		// 0x0F 06 YY MM DD hh mm ss Le SS 00 00 00 00 00 00 00 00 00 00
		
		unsigned char txbuf[20];
		
		memset( txbuf, 0, sizeof(txbuf));
		
		txbuf[0] = 0x0F;
		txbuf[1] = 0x06;
	
		txbuf[2] = TimerYY();
		txbuf[3] = TimerMM();
		txbuf[4] = TimerDD();
		txbuf[5] = TimerHH();
		txbuf[6] = TimerMI();
		txbuf[7] = TimerSE();
		txbuf[8] = power.battery_level;
		txbuf[9] = power.charge_flage;
		txbuf[10]= (power.time_after_last>>16) & 0xFF ;
		txbuf[11]= (power.time_after_last>>8) & 0xFF;
		txbuf[12]= power.time_after_last & 0xFF;
		txbuf[13]= power.compelete;
		txbuf[14]= current_adc>>8;
		txbuf[15]= current_adc&0xFF;
		
		bt_protocol_tx(txbuf, sizeof(txbuf));
		
		return ( events ^ TASK_BATTERY_QUERY_EVT );
	}
	
	if ( events & TASK_MINITE_TIMEOUT_EVT )
	{
		power.time_after_last++;
		
		return ( events ^ TASK_MINITE_TIMEOUT_EVT );
	}
	
	if ( events & TASK_BATTERY_INIT_EVT )
	{	
		osal_set_event( task_id, TASK_BATTERY_DECT_EVT );
		
		osal_start_timerEx( task_id, TASK_BATTERY_FEED_ARRAY_EVT, 100);
		
		osal_start_timerEx( task_id, TASK_BATTERY_CHARGE_EVT, 1000);	


		return ( events ^ TASK_BATTERY_INIT_EVT ); 
	}
	
	return 0;
}

void taskBatteryInit( unsigned long task_id )
{
	taskBatteryTaskId = task_id;	
	
	nrf_gpio_pin_pull_t config1 = NRF_GPIO_PIN_PULLUP;
	nrf_gpio_cfg_input(BAT_STAT_DET_PIN, config1);

	/** GPIOTE0 chanenl 1 as event
	 *  for GPIO pin BAT_STAT_DET_PIN (3)
	 *  when any change on pin interrupt
	 */
	NRF_GPIOTE->CONFIG[2] = (1<<0) |
					 (BAT_STAT_DET_PIN << 8) |
					 (3 << 16);
	// Enable interrupt
	NRF_GPIOTE->INTENSET |= 0x1UL<<2;
	
	
	nrf_gpio_pin_pull_t config2 = NRF_GPIO_PIN_NOPULL;
	nrf_gpio_cfg_input(BAT_CHARGE_PIN, config2);

	/** GPIOTE0 chanenl 1 as event
	 * for GPIO pin BAT_CHARGE_PIN (6)
	 *  when any change on pin trigger interrupt
	 */
	NRF_GPIOTE->CONFIG[3] = (1<<0) |
					 (BAT_CHARGE_PIN << 8) |
					 (3 << 16);
	// Enable interrupt
	NRF_GPIOTE->INTENSET |= 0x1UL<<3;
	
	battery_flag = 0;	
	config.motor_test_enable = 1;
	
	osal_set_event ( task_id, TASK_BATTERY_INIT_EVT );
}

