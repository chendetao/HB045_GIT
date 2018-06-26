
#include "string.h"

#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_Step.h"
#include "Task_Upload.h"
#include "Task_GUI.h"
#include "Task_Ble.h"
#include "Task_Battery.h"
#include "Task_Finder.h"

#include "btprotocol.h"
#include "ClockTick.h"
#include "Alarm.h"
#include "Drink.h"
#include "Medicine.h"
#include "UserSetting.h"
#include "version.h"
#include "MsgQueue.h"
#include "Ancs.h"
#include "Log.h"
#include "Window.h"
#include "UI_win_notify.h"
#include "Task_GUI.h"
#include "Task_Motor.h"
#include "Task_Flash.h"
#include "Task_Hr.h"
#include "Task_UItraviolet.h"
#include "Task_Store.h"

extern int ble_write_bytes( unsigned char *buf, int length );

static int handle_notification( const unsigned char *buf, unsigned char *txbuf);

int bt_protocol_tx( unsigned char *buf, int length )
{
	return ble_write_bytes(buf, length);
}

const unsigned char lcd_level[3] = {0x00,0x48,0xDC};
extern bool font_is_ok(void);

int bt_protocol_rx(const unsigned char *buf, int length )
{
	unsigned char txbuf[20];
	unsigned int temp;	
	
	memset( txbuf, 0, sizeof(txbuf) );
	
	txbuf[0] = buf[1];
	
    // 设备防丢/查找指令
    if ( (buf[0] == 0x10 ) && (buf[1] == 0x1) ) /* 收到手机确认 */
    {
        /* 没有超过时限才有效 */
        if ( finder.state == FIND_PHONE_STATE_FINDING ){
            finder.state = FIND_PHONE_STATE_STOP;
            finder.isFound = 1;
        }
        return 0 ;
		
    }	
	
	switch ( buf[1] )
	{
	case 0: /* Request SET/GET system date and time */
			// 0xFC 00 YY MM DD hh mm ss We 00 00 00 00 00 00 00 00 00 00 00
			config.sync_flag |= 0x1<<0;
            if ( SetTimer( buf[5],buf[6],buf[7] ) 
                || SetDate(buf[2],buf[3],buf[4],buf[8]) )
            {
                txbuf[0] |= (0x1<<7);
            }else{
                memcpy(txbuf+1, buf+2, 7);
            }break;		
	case 1: /* Request SET/GET system alarm(s) */
			// 0xFC 01 EN A1 A2 A3 A4 A5 00 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[2] == 0x1 )
            {
                txbuf[1] = buf[2];
                
                txbuf[2] = (alarm.alarm_s>>0) & 0x3;
                txbuf[3] = (alarm.alarm_s>>6) & 0x3;
                txbuf[4] = (alarm.alarm_s>>12) & 0x3;
                txbuf[5] = 0;
                txbuf[6] = 0;
                
                txbuf[7]  = DECToBCD(H(0));
                txbuf[8]  = DECToBCD(M(0));
                
                txbuf[9]  = DECToBCD(H(3));
                txbuf[10] = DECToBCD(M(3));
                
                txbuf[11] = DECToBCD(H(6));
                txbuf[12] = DECToBCD(M(6));               
            }else{
                if ( SetAlarm( buf+3 ) )
                {
                    txbuf[0] |= (0x1<<7);
                }else{
                    memcpy(txbuf+1, buf+2, 17);   
                    calc_next_Alarm(0);
                }
            }break;	
	case 2: /* Request GET/SET Motor viborate Enable state */
			// 0xFC 02 EN 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[2] == 0x1 )
            {
                config.viborate_enable = 1;
				
				osal_set_event( taskMotorTaskId, TASK_MOTOR_BLE_CONNECT_EVT );
				
            }else if( buf[2] == 0x0 ){
                config.viborate_enable = 0;
            }else{
                txbuf[0] |= (0x1<<7);
            }break;
    case 3: /* Request Current/History Pedometer Distance Calorie */
			// 0xFC 03 EN 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( ( buf[2] == 0x80 ) )     	// Request history items
            {
                txbuf[1] = 0x80;
                txbuf[2] = fm.erea[FM_ID_PEDOMETER].items;
            }else if ( (buf[2] == 0xC0 )) 	// Request history date
            {
                osal_set_event( taskUploadTaskId, TASK_UPLOAD_STORE_PEDO_EVT );
                return 0;
            }
        
            if ( ( buf[2] >> 0) & 0x1)  	// Pedometer Counter
            {
                txbuf[1] |= 0x1<<0;
                      
                txbuf[2] = (pedometer.counter >>16) & 0xFF;
                txbuf[3] = (pedometer.counter >> 8) & 0xFF;
                txbuf[4] = (pedometer.counter >> 0) & 0xFF;                      
            }
            if ( ( buf[2] >> 1) & 0x1)  	// Distance
            {
                txbuf[1] |= 0x1<<1;
                      
                txbuf[5] = (pedometer.distance >>16) & 0xFF;
                txbuf[6] = (pedometer.distance >> 8) & 0xFF;
                txbuf[7] = (pedometer.distance >> 0) & 0xFF;                      
            }
            if ( (buf[2] >> 2 ) & 0x1)   	// Calorie
            {
                txbuf[1] |= 0x1<<2;
                      
                txbuf[8] = (pedometer.calorie >>16) & 0xFF;
                txbuf[9] = (pedometer.calorie >> 8) & 0xFF;
                txbuf[10]= (pedometer.calorie >> 0) & 0xFF;                      
            }
            break;
    case 0x4: /* Request Clear current sport data */
			// 0xFC 04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
			osal_set_event( taskStepTaskId, TASK_STEP_CLEAR_EVT );
            return 0;
    case 0x6: /* Request SET/SET user body informations (optional) */
			// 0xFC 06 WE HE SX YE 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[2] == 0 || buf[3] == 0x0 )
            {
                txbuf[0] |= 0x1<<7;
            }else{
                config.weight = buf[2];
                config.height = buf[3];
                          
                txbuf[1] = buf[2];
                txbuf[2] = buf[3];
            }
            break;               
    case 0x7: /* Request SET/GET Pedometer goal */
			// 0xFC 07 Ty S2 S1 S0 G2 G1 G0 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[2] & 0x1 )
            {
                pedometer.goal = (((unsigned long)buf[3])<<16) + 
						(((unsigned long)(buf[4]))<<8) + buf[5];
            }else{
                txbuf[0] |= 0x1<<7;
            }break;
	case 0x8: /* Request SET/GET the Notificaton configuration or notify */
			if ( handle_notification(buf, txbuf) != 0)
			{
				return (-1); /* (-1) indicate we will send a security request */
			}
			break;
	case 0x9: /* Heart Rate Function */
			// 0xFC 0A Ty 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
			if ( buf[2] == 0x0 )
			{
				osal_set_event ( taskGUITaskId, TASK_GUI_HR_STOP_EVT );
				txbuf[1] = 0x00;
			}else if ( buf[2] == 0x1 )  /* */
			{
				txbuf[0] |= 0x1<<7; /* Not supported this mode now */
			}else if ( buf[2] == 0x2 )  /* SINGLE */
			{           
				osal_set_event ( taskGUITaskId, TASK_GUI_HR_START_EVT );
				
				// Stop GSensor upload
				config.gsensor_debug = 0;				
				
				txbuf[1] = 0x02;
			}else{
				txbuf[0] |= 0x1<<7;
			}
			break;			
    case 0xA: /* Request heart rate */
			// 0x0A 00 00 00 00 00 YY MM DD hh mm ss Hr 00 00 00 00 00 00 00
			if ( buf[2] == 0x0 ) 		// Recently Data
			{
				osal_set_event( taskUploadTaskId, TASK_UPLOAD_HR_EVT );
				return 0;
			}else if ( buf[2] == 0x1 ) 	// History Data
			{
				osal_set_event( taskUploadTaskId, TASK_UPLOAD_STORE_HR_EVT );				
				return 0;
			}else if ( buf[2] == 0x2 ) 	// History Data Items
			{
				txbuf[1] = 0x02;
				txbuf[2] = fm.erea[FM_ID_HR].items / 256;
				txbuf[3] = fm.erea[FM_ID_HR].items % 256;
			}else{
				txbuf[0] |= 0x1<<7;
			}
            break;
    case 0xC: /* Request sleep data */
			// 0xFC 0C Ty 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[2] == 0x0 )  		// Recently Data
            {
                osal_set_event( taskUploadTaskId, TASK_UPLOAD_SL_EVT );
                return 0;
                
            }else if ( buf[2] == 0x01 ) // History Data
            {
                osal_set_event( taskUploadTaskId, TASK_UPLOAD_STORE_SL_EVT );
                return 0;
            }else if ( buf[2] == 0x02 ) // History Data Items
            {
				txbuf[1] = 0x02;
				txbuf[2] = fm.erea[FM_ID_SLEEP].items;                
            }else if( buf[2] == 0x03 )
			{
				osal_set_event( taskStepTaskId, STEP_TASK_TEST_EVT );
			}
            break;	
    case 0x10: /* Request SET/GET the notification function */
			// 0xFC 10 02 TT CC 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[2] == 0x2 )
            {
                if ( buf[3] == 1)		// Enable notification
                {
                    config.notify_enable = 1;
                    config.delay_timeout = buf[4]; // Timeout value
                }else               	// Disable notification
                {
                    config.notify_enable = 0;
                    config.delay_timeout = 0;
                }
            }else if ( buf[2] == 0x3 ) 	// Get Notification Configuration.
            {
                txbuf[1] = 0x03;
                txbuf[2] = config.notify_enable;
                txbuf[3] = config.delay_timeout;
            }else
            {
                txbuf[0] |= 0x1<<7;
            }
            break;	
    case 0x11: // Request blood pressure ( handle as heart rate )
		    // 0xFC 11 Ty Ch Cl 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[2] == 0x0 )
            {
                osal_set_event( taskUploadTaskId, TASK_UPLOAD_B_PRESSURE_EVT );
                return 0;                
            }else if ( buf[2] == 0x1 )
            {
                osal_set_event( taskUploadTaskId, TASK_UPLOAD_STORE_B_P_EVT );
                return 0;                
            }else if ( buf[2] == 0x2 )
            {
                txbuf[1] = 0x02;
                txbuf[2] = fm.erea[FM_ID_B_PRESSURE].items / 256;
                txbuf[3] = fm.erea[FM_ID_B_PRESSURE].items % 256;                
            }else
            {
                txbuf[0] |= 0x1<<7;
            }
            break;           
    case 0x12: // Request SPO2 (handle as heart rate )
            if ( buf[2] == 0x0 )
            {
                osal_set_event( taskUploadTaskId, TASK_UPLOAD_SPO2_EVT );
                return 0;                
            }else if ( buf[2] == 0x1 )
            {
                osal_set_event( taskUploadTaskId, TASK_UPLOAD_STORE_SPO2_EVT );
                return 0;                
            }else if ( buf[2] == 0x2 )
            {
                txbuf[1] = 0x02;
                txbuf[2] = fm.erea[FM_ID_SPO2].items / 256;
                txbuf[3] = fm.erea[FM_ID_SPO2].items % 256;                
            }else
            {
                txbuf[0] |= 0x1<<7;
            }
            break;

    case 0x14: // Factory Test Instruction
            if ( buf[2] == 0x0 )
            {
                txbuf[1] = 0x1;
                txbuf[2] = TimerYY();
                txbuf[3] = TimerMM();
                txbuf[4] = TimerDD();
                txbuf[5] = TimerHH();
                txbuf[6] = TimerMI();
                txbuf[7] = TimerSE();
                txbuf[8] = TimerWK();
                
                config.gsensor_debug = 1;
				
				if ( buf[3] == 0x1 ){
					osal_set_event( taskMotorTaskId, TASK_MOTOR_BMA_DONE_EVT ); 
				}
            }else{
                txbuf[0] |= 0x1<<7;
            }break;			
			
    case 0x15:/* SET/GET the LCD behavior when turn-over the hand */
			// 0xFC 15 SS 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( ( buf[2]>>7) & 0x1 )
            {
                txbuf[1] = config.control & BSTOTW_FLAG_ENABLE;
            }else
            {
                if ( buf[2] & 0x1) /* Enable function */
                {
                    config.control = 1;                   
                }else{             /* Disable function */
                    config.control = 0;
                }
                txbuf[1] = config.control;
            }
            break;
    case 0x16:/* Sedentary Notification Configure */
            // 0xFC 16 SS T2 T1 SH SM EH EM IH IM Ih Im PH PL 00 00 00 00 00
		
			if ( (buf[2]>>7) & 0x1 )
			{
				txbuf[1]  = buf[2];
				txbuf[2]  = monitor.ss;
				txbuf[3]  = monitor.interval;
				txbuf[4]  = DECToBCD(monitor.start/60);
				txbuf[5]  = DECToBCD(monitor.start%60);
				txbuf[6]  = DECToBCD(monitor.end/60);
				txbuf[7]  = DECToBCD(monitor.end%60);
				txbuf[8]  = DECToBCD(monitor.sedentary_start/60);
				txbuf[9]  = DECToBCD(monitor.sedentary_start%60);
				txbuf[10] = DECToBCD(monitor.sedentary_end/60);
				txbuf[11] = DECToBCD(monitor.sedentary_end%60);	
				txbuf[12] = monitor.pedometer>>8;
				txbuf[13] = monitor.pedometer&0xFF;
				txbuf[14] = monitor.last_pedometer>>8;
				txbuf[15] = monitor.last_pedometer&0xFF;
				break;
			}
	
            if ( !( buf[2] & 0x1 ) || !(buf[4] > 0 ) ){
				monitor.ss = 0x0;
				break;
			}
				
			temp = (unsigned short)(buf[3]<<8) + buf[4];/* 提醒间隔1-255分钟 */
			if ( temp > 255 || temp == 0 ){
				txbuf[0] = 0x1<<7;
				break;
			}		
			temp = (unsigned short)(buf[13]<<8) + buf[14];
			if ( temp == 0x0 ){
				txbuf[0] = 0x1<<7;
				break;
			}
			
			monitor.pedometer = temp;
			monitor.ss = buf[2] & 0x3;
			
			/** Set sedentary notification time interval */
			monitor.start = BCDToDEC(buf[9]) *60 + BCDToDEC(buf[10]);
			monitor.end = BCDToDEC(buf[11]) *60 + BCDToDEC(buf[12]);
			
			monitor.interval = buf[4];
			osal_set_event( taskStepTaskId, TASK_STEP_SED_INIT_EVT );
			
			/** Set Don't disturb time interval if set */
			if ( (monitor.ss >> 1) & 0x1)
			{
				monitor.sedentary_start = BCDToDEC(buf[5]) *60 + BCDToDEC(buf[6]);
				monitor.sedentary_end = BCDToDEC(buf[7]) *60 + BCDToDEC(buf[8]);
			}               
			
			memcpy(txbuf+1, buf+2, 13);
			monitor.last_pedometer = pedometer.counter;
            
            break;		
	case 0x0F: /* Firmware maintenance */
		    // 0x0F 05 YY MM DD 00 00 MA MI RE 55 DH DL 00 00 00 00 00 00 00
			if ( buf[2] == 0x01 )
			{
				osal_start_timerEx( taskBleTaskId, TASK_BLE_SYSTEM_REBOOT_EVT,500 );
			}else
			if ( buf[2] == 0x04 )
			{
				/** Set Lcd level */
				config.lcd_level_r = lcd_level[buf[3]%3];
				config.lcd_level_g = lcd_level[buf[3]%3];
				config.lcd_level_b = lcd_level[buf[3]%3];
				osal_set_event ( taskGUITaskId, TASK_GUI_LCD_LEVEL_UPDATE_EVT);
			}else
			if ( buf[2] == 0x05 )
			{
				/** Get Firmwave Version Informations */
				txbuf[1] = 0x05;
				txbuf[2] = VERSION_YEAR;
				txbuf[3] = VERSION_MONTH;
				txbuf[4] = VERSION_DAY;
				txbuf[7] = VERSION_MAJOR;
				txbuf[8] = VERSION_MINOR;
				txbuf[9] = VERSION_REVISON;
				txbuf[10] = 0x55;
				txbuf[11]= PRODUCT_ID_H;
				txbuf[12]= PRODUCT_ID_L;
				config.sync_flag |= 0x1<<1;
			}else
			if ( buf[2] == 0x06 )
			{
				/** Request Battery Level */
				osal_set_event( taskBatteryTaskId, TASK_BATTERY_QUERY_EVT );
				if ( config.sync_flag == 0x3 )
				{
					if ( config.lcdEnable == 0 || win_current == 1 )
					{
						osal_set_event( taskGUITaskId, TASK_GUI_TOUCH_EVT );
					}
				}					
				config.sync_flag = 0;				
				return 0;
			}else
			if ( buf[2] == 0x0B )
			{
				/** Restore factory settings */
				osal_set_event ( taskFlashTaskId, TASK_FLASH_MAKE_TO_DEFAULT_EVT );
				return 0;
			}else
			if ( buf[2] == 0x0F )
			{
				// Request Get the Hr wave 
				if ( buf[3] == 1 )
				{
					config.hrWaveUploadEnable = buf[4] & 0x1;
				}else{
                    txbuf[0] |= 0x1<<7;
				}
				txbuf[1] = 0x0F;
				txbuf[2] = buf[3];					
			}else
			if ( buf[2] == 0x10 )
			{
				unsigned long cnt = (((unsigned long)buf[3])<<16) 
					| (((unsigned long)buf[4])<<8) | buf[5];
				load_pedometer(cnt);
			}else if ( buf[2] == 0x11 ) /** Font comfirm */
			{
				txbuf[1] = 0x11;
				txbuf[2] = font_is_ok();
			}else if ( buf[2] == 0x12 ) /** Enable Gsensor Wave upload */
			{
				config.gsensor_debug = buf[3] & 0x1;
				config.gsensor_fast_enable = buf[4] & 0x1;
				txbuf[1] = 0x12;				
			}else if ( buf[2] == 0x13 ) /** GSensor comfirm */
			{
				osal_set_event( taskStepTaskId, TASK_STEP_MONITOR_COMFIRM_EVT );
				txbuf[1] = 0x13;				
			}else if ( buf[2] == 0x14 ) /** Set heart rate touch level */
			{
				if ( buf[3] == 0 )
				{
					hr.touch_level = ( (buf[4]<<8)|buf[5] ) & 0xFFF;
				}else{
					txbuf[3] = hr.touch_level>>8;
					txbuf[4] = hr.touch_level & 0xFF;
				}
				txbuf[1] = 0x14;
				txbuf[2] = buf[3];
			}			
			else if ( buf[2] == 0x16 ) /** power off */		
			{
				osal_start_timerEx( taskGUITaskId, TASK_GUI_SYS_POWEROFF_EVT,500);
				txbuf[1] = 0x16;
			}else if ( buf[2] == 0x1A ){
				osal_set_event ( taskFlashTaskId, TASK_FLASH_RESET_EVT );
				return 0;
			}else
			{
				txbuf[0] |= 0x1<<7;
			}
			break;
	case 0x17: /** Unit Set/Get */
			// 0xFC 17 00 SS 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[3] == 0 )
            {
               config.unit = 0;
               txbuf[2] = 0;
            }else if ( buf[3] == 1 )
            {
               config.unit = 1;
               txbuf[2] = 1;
            }else
            {
              txbuf[0] |= (0x1<<7);
            }
			
			unit_update();
			osal_set_event( taskUploadTaskId, TASK_UPLOAD_PEDO_EVT);
			
			break;
	case 0x18: /** Time Format Set/Get */
			// 0xFC 18 00 SS 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[3] == 0 )
            {
               config.time_format = 0;
               txbuf[2] = 0;
            }else if ( buf[3] == 1 )
            {
               config.time_format = 1;
               txbuf[2] = 1;
            }else
            {
              txbuf[0] |= (0x1<<7);
            }
			
			break;			
	case 0x19: /** Take photo */
			// 0xFC 19 EN ST 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	
			memcpy( txbuf, buf+1, 5);
			if ( buf[2] == 0x81 )
			{
			  osal_set_event ( taskGUITaskId, TASK_GUI_TAKE_PHOTO_EVT );
			}else if ( buf[2] == 0x80 )
			{
			  osal_set_event ( taskGUITaskId, TASK_GUI_EXIT_PHOTO_EVT );
			}else if ( (buf[2] == 0x00) && (buf[3] == 0x80) )
			{	
			  osal_set_event ( taskMotorTaskId, TASK_MOTOR_ACTION_DONE_EVT );				
			}else
			{
				txbuf[0] |= 0x1<<7;
			}
			break;
	case 0x1A: /** Get segment pedometer data */
			// 0xFC 1A EN 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
			if ( buf[2] == 0x2 )
			{
                txbuf[1] = 0x02;
				unsigned long cnt = fm.erea[FM_ID_SEG_PEDO].items;
				cnt <<= 12;
                txbuf[2] = (cnt>>16) & 0xFF;
                txbuf[3] = (cnt>> 8) & 0xFF;
				txbuf[4] = (cnt>> 0) & 0xFF;
			}else if ( buf[2] == 0x4 )
			{
				osal_set_event( taskUploadTaskId, TASK_UPLOAD_STORE_SEG_PEDO_EVT);
				return 0;
			}else{
				txbuf[0] |= 0x1<<7;
			}
			break;
	case 0x1B: /** Get Running Data (All Data Null now)*/
            // 0xFC 1B EN 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
            if ( buf[2] == 0x0 )
            {        
              txbuf[1] = 0x0;
            }else if ( buf[2] == 0x1 )
            {
               osal_set_event( taskUploadTaskId, TASK_UPLOAD_STORE_RUNNING_EVT );  
               return 0;
            }else if ( buf[2] == 0x2 ){
               txbuf[1] = 0x02; // EN
               txbuf[2] = 0x00; // SS
               txbuf[3] = fm.erea[FMC_ID_RUNNING].items>>4;
               txbuf[4] = (fm.erea[FMC_ID_RUNNING].items & 0xF)<<4;
            }else{
               txbuf[1] = 0x1<<7;
            }
            break;			
	case 0x1C: /** Get/Set the window */
			if ( buf[2] == 0x1 )
			{
				/** Get Device UI windows numbers */
				txbuf[1] = 0x1;
				txbuf[2] = 0xA; // we have 10 windows can provide to user
			}else if ( buf[2] == 0x2 ){
				txbuf[1] = 02;
				#if 0
				if ( buf[3] == 0x1 )
				{
					// Set window
					window_set( buf[4]<<8 | buf[5], buf+6);
					txbuf[2] = 0x1;
				}else{
					// Get window
					// Not support now
					txbuf[0] |= 0x1<<7;
				}
				#else
				txbuf[2] = 0x1;
				#endif
			}else{
				txbuf[0] |= 0x1<<7;
			}
			break;	
	case 0x22: /* Automatic heart rate measurement configuration */
			if ( buf[2] == 0x01 )   	// OP
			{
				if ( buf[3] == 0x0 ) 	// EN
				{
					hr.auto_measure_enable = buf[4] & 0x1;
					
					hr.auto_measure_interval = ((unsigned long)buf[5]<<8 | buf[6]) & 0xFFF;
					if ( hr.auto_measure_interval < 5 )
					{
						hr.auto_measure_enable = 0;
						txbuf[0] |= 0x1 << 7;
					}else{
						osal_start_timerEx ( taskHrTaskId,\
						TASK_HR_DECT_AUTO_EVT,\
						hr.auto_measure_interval*60*1000);
					}
				}else{
					txbuf[1] = 01;
					txbuf[2] = 01;
					txbuf[3] = hr.auto_measure_enable;
					txbuf[4] = (hr.auto_measure_interval>>8) & 0xF;
					txbuf[5] = hr.auto_measure_interval & 0xFF;
				}
			}else{
				txbuf[0] |= 0x1 << 7;
			}
			break;	
    case 0x28:
            if ( buf[2] == 0x0 )          // 查询当前(最近一次)
            {
                txbuf[1] = 0x00;
                txbuf[2] = (int)UIT_i[0];
            }else if ( buf[2] == 0x1 ){   // 获取历史记录
                osal_set_event ( taskStoreTaskId, TASK_UPLOAD_STORE_UIT_EVT);
                return 0;
            }else if ( buf[2] == 0x2 )    // 获取历史记录条数
            {
                txbuf[1] = 0x2;
                txbuf[2] = fm.erea[FMC_ID_UIT].items & 0xFF;                
            }else if ( buf[2] == 0x80 )   // 打开或关闭数据上报功能.
            {
                uit_notify_enable = buf[3] & 0x1;
                txbuf[1] = 0x80;
                txbuf[2] = uit_notify_enable;                
            }else{
                txbuf[0] |= 0x1<<7;
            }
            break;
            
    case 0x29:  /** 天气推送 */
            if ( buf[2] == 0x1 )
            {
                fm.weather.yy = TimerYY();
                fm.weather.mm = TimerMM();
                fm.weather.dd = TimerDD();
                fm.weather.hh = TimerHH();
                fm.weather.tt = buf[3]; // 天气类型
                fm.weather.ht = buf[4]; // 最高气温
                fm.weather.lt = buf[5]; // 最低气温
                fm.weather.ct = buf[6]; // 当前气温
                txbuf[1] = buf[2];
                txbuf[2] = buf[3];
                txbuf[3] = buf[4];
                txbuf[4] = buf[5];
            }else{
                txbuf[0] |= 0x1<<7;
            }
            break;
    case 0x2B:  /** 吃药提醒 */
            if ( buf[2] == 0x1 )
            {
                txbuf[1] = buf[2];
                
                txbuf[2] = (medicine.alarm_s>>0) & 0x3;
                txbuf[3] = (medicine.alarm_s>>6) & 0x3;
                txbuf[4] = (medicine.alarm_s>>12) & 0x3;
                txbuf[5] = 0;
                txbuf[6] = 0;
                
                txbuf[7]  = DECToBCD(MMH(0));
                txbuf[8]  = DECToBCD(MMM(0));
                
                txbuf[9]  = DECToBCD(MMH(1));
                txbuf[10] = DECToBCD(MMM(1));
                
                txbuf[11] = DECToBCD(MMH(2));
                txbuf[12] = DECToBCD(MMM(2));  
                
                txbuf[13] = DECToBCD(MMH(3));
                txbuf[14] = DECToBCD(MMM(3));
                
                txbuf[15] = DECToBCD(MMH(4));
                txbuf[16] = DECToBCD(MMM(4));                
            }else{
                if ( SetMedicine( buf+3 ) )
                {
                    txbuf[0] |= (0x1<<7);
                }else{
                    memcpy(txbuf+1, buf+2, 17);
                }
                calc_next_Medicine(0);
            }        
            break;
    case 0x2C:  /** 喝水提醒 */
            if ( buf[2] == 0x1 )
            {
                txbuf[1] = buf[2];
                
                txbuf[2] = (drink.alarm_s>>0) & 0x3;
                txbuf[3] = (drink.alarm_s>>6) & 0x3;
                txbuf[4] = (drink.alarm_s>>12) & 0x3;
                txbuf[5] = 0;
                txbuf[6] = 0;
                
                txbuf[7]  = DECToBCD(DH(0));
                txbuf[8]  = DECToBCD(DM(0));
                
                txbuf[9]  = DECToBCD(DH(1));
                txbuf[10] = DECToBCD(DM(1));
                
                txbuf[11] = DECToBCD(DH(2));
                txbuf[12] = DECToBCD(DM(2)); 

                txbuf[13] = DECToBCD(DH(3));
                txbuf[14] = DECToBCD(DM(3));

                txbuf[15] = DECToBCD(DH(4));
                txbuf[16] = DECToBCD(DM(4));                
            }else{
                if ( SetDrink( buf+3 ) )
                {
                    txbuf[0] |= (0x1<<7);
                }else{
                    memcpy(txbuf+1, buf+2, 17);   
                }
                calc_next_Drink(0);                
            }          
            break;
    case 0x2D:  /** APP通知一键开启/关闭 */
            if ( buf[2] == 0x01 )
            {
                txbuf[1] = config.app_all_enable;
            }else
            {
                config.app_all_enable = buf[3] & 0x1;
                txbuf[1] = buf[2];
                txbuf[2] = buf[3];
            }
            break;
	default: /** Unspported Instruction */
			txbuf[0] |= 0x1<<7;
			break;
	}
	
	if ( buf[0] == 0xFC && buf[1] == 0x10 && buf[2] == 0x00 )
    {
        if ( buf[3] == 0x0 )/* 手机取消查找 */
        {
            if ( finder.phoneFindMe == 1 )
            {
                finder.phoneFindMe = 0;
                osal_set_event( taskFinderTaskId, FINDPHONE_TASK_FINDME_STOP_EVT );
            }
        }else               /* 手机开始查找 */
        {
            finder.phoneFindMe = 1;
            osal_set_event( taskFinderTaskId, FINDPHONE_TASK_FINDME_START_EVT );
        }
                
        return 0;
    }	
	
	/**
	 * Make a response to client
	 */
	bt_protocol_tx( txbuf, sizeof(txbuf));
	
	return 0;
}

void notify_remove(void);

int handle_notification( const unsigned char *buf, unsigned char *txbuf)
{
	int rval = 0;
	static unsigned char id = 0;
	
	if ( buf[2] == 0xF0 )
	{
		/** 打开ANCS通知提醒功能(For iOS).
		 */                
		ble.ancs_notify_enable = buf[3] | buf[4]<<8 | buf[5]<<16 | buf[6]<<24;
		
		if ( ble.ancs_notify_enable != 0 )
		{
			slave_security_request();
			return (1);
		}
		
		txbuf[0] = 0x08;
		txbuf[1] = ble.ancs_notify_enable&0xFF;
		txbuf[2] = (ble.ancs_notify_enable>>8)&0xFF;
		txbuf[3] = (ble.ancs_notify_enable>>16)&0xFF;
		txbuf[4] = (ble.ancs_notify_enable>>24)&0xFF;
		txbuf[5] = 0x00;
		
		return (0);
	}
	
	if ( buf[2] == 0xF1)
	{
		/** 获取ANCS通知提醒配置状态(For iOS).
		 * Todo: make a response to central device
		 */
		txbuf[0] = 0x08;
		txbuf[1] = 0xF1;
		txbuf[2] = ble.ancs_notify_enable;
		
		return 0;
	}	
    
    ancs_ancs_flag = 0;    
	
	/**
	 * 来电、短信、APP消息推送处理(For Android)
	 */
	memcpy( txbuf, buf+1, 5);    
			
	if ( buf[2] == 0 )  /* 来电推送 */
	{
		if ( buf[3] == 0x0 ) /* 仅仅推送通知 */
		{
			NOTICE_ADD(NOFIFY_ID_CALLING );
			MessageClearAll();
		}else if ( buf[3] == 0x1 )
		{
			MessageClearAll();
			/* 推送联系人姓名(1) */
			rval = MessageBuildSender(id++, (buf[4] > 12) ? (12) : buf[4], buf+5);
			NOTICE_ADD(NOFIFY_ID_CALLING );
			mq.type = NOTIFY_TYPE_CALL;
		}else if ( buf[3] == 0x2 )
		{
			/* 推送电话号码(2) 
			 * 支持11位数显示,多余的暂时不显示
			 */
					
			/** 
			 * 来电电话号码从BCD格式编码转换为Unicode
			 */
			unsigned char sender_len, unicode_buf[2],i = 0;
			sender_len = (buf[4] > 11) ? (11) : buf[4];
			while( sender_len >0 )
			{
				unicode_buf[1] = (buf[5+i]>>4) + '0';
				unicode_buf[0] = 0x0;
				rval = MessageBuildSender(id, 2, unicode_buf);
				 
				sender_len--;
				if ( sender_len == 0 ){
					break;
				}
				  
				unicode_buf[1] = (buf[5+i] & 0xF) +'0';
				unicode_buf[0] = 0x0;
				  
				rval = MessageBuildSender(id, 2, unicode_buf);
				sender_len--; i++;
			}
					  
			id++;
					  
			NOTICE_ADD(NOFIFY_ID_CALLING );
			mq.type = NOTIFY_TYPE_CALL;			
		}else{
			txbuf[0] = 0x1<<7;
		}
	}else if ( buf[2] == 1 )
	{
		/**
		 * Android 短信推送 */
		if ( buf[3] == 0x0 )
		{
			MessageClearAll(); /** 仅仅通知 */
			NOTICE_ADD(NOTIFY_ID_EMAIL );
		}else
		if ( ((buf[3]>>7) & 0x1) == 0x1 ) 
		{
		   /* 发信人 */
		   MessageClearAll();
		   if ( (buf[3]>>6) & 0x1){ 
			   // 发信人为通讯录里面的名字
			   MessageBuildSender(buf[3]&0x3F,  buf[4], buf+6);
		   }else
		   {
			  /** 未知发信人、发信人为电话号码
			   *  将BCD格式编码的电话号码转换为Unicode
			   */
			  unsigned char sender_len, unicode_buf[2],i = 0;
			  sender_len = buf[4]; // 单位：字符个数
			  while ( sender_len > 0 )
			  {
				  unicode_buf[1] = (buf[6+i]>>4) + '0';
				  unicode_buf[0] = 0x0;
				  rval = MessageBuildSender(buf[3]&0x3F, 2, unicode_buf);
				 
				  sender_len--;
				  if ( sender_len == 0 ) {
					  break;
				  }
				  
				  unicode_buf[1] = (buf[6+i] & 0xF) +'0';
				  unicode_buf[0] = 0x0;
				  
				  rval = MessageBuildSender(buf[3] & 0x3F, 2, unicode_buf);
				  sender_len--; i++;
			  }
			}	
		   
		   if ( buf[4] > 0 )
		   {
				NOTICE_ADD(NOTIFY_ID_EMAIL );			
				mq.type = NOTIFY_TYPE_MSG;
		   }                       
		}else
		{
			/* 短信内容 */
			rval = MessageBuildContent(buf[3]&0x3F, buf[4], buf+6);
		}
		
		if ( rval != 0 )
		{
			txbuf[0] |= 0x1<<7;
		}
	}else if ( buf[2] == 0x2 ){
		/**
		 * Android APP (QQ) 消息推送 */
		if ( buf[3] == 0x0 )
		{
			MessageClearAll(); /** 仅仅通知 */
			NOTICE_ADD(NOTIFY_ID_APP_MSG );
		}else
		if ( ((buf[3]>>7) & 0x1) == 0x1 ) 
		{
		   /* 发信人 */
		   MessageClearAll();
		   if ( (buf[3]>>6) & 0x1){ 
			   // 发信人为通讯录里面的名字
			   MessageBuildSender(buf[3]&0x3F,  buf[4], buf+6);
		   }else
		   {
			  /** 未知发信人、发信人为电话号码
			   *  将BCD格式编码的电话号码转换为Unicode
			   */
			  unsigned char sender_len, unicode_buf[2],i = 0;
			  sender_len = buf[4]; // 单位：字符个数
			  while ( sender_len > 0 )
			  {
				  unicode_buf[1] = (buf[6+i]>>4) + '0';
				  unicode_buf[0] = 0x0;
				  rval = MessageBuildSender(buf[3]&0x3F, 2, unicode_buf);
				 
				  sender_len--;
				  if ( sender_len == 0 ) {
					  break;
				  }
				  
				  unicode_buf[1] = (buf[6+i] & 0xF) +'0';
				  unicode_buf[0] = 0x0;
				  
				  rval = MessageBuildSender(buf[3] & 0x3F, 2, unicode_buf);
				  sender_len--; i++;
			  }
			}	
		   
		   if ( buf[4] > 0 )
		   {
				NOTICE_ADD(NOTIFY_ID_APP_MSG );
				mq.type = NOTIFY_TYPE_QQ;
		   }                       
		}else
		{
			/* QQ内容 */
			rval = MessageBuildContent(buf[3]&0x3F, buf[4], buf+6);
		}
		
		if ( rval != 0 )
		{
			txbuf[0] |= 0x1<<7;
		}
	}else if ( buf[2] == 0x4 )
	{
		/**
		 * Android APP (微信) 消息推送 */
		if ( buf[3] == 0x0 )
		{
			MessageClearAll(); /** 仅仅通知 */
			NOTICE_ADD(NOTIFY_ID_APP_MSG );
		}else
		if ( ((buf[3]>>7) & 0x1) == 0x1 ) 
		{
		   /* 发信人 */
		   MessageClearAll();
		   if ( (buf[3]>>6) & 0x1){ 
			   // 发信人为通讯录里面的名字
			   MessageBuildSender(buf[3]&0x3F,  buf[4], buf+6);
		   }else
		   {
			  /** 未知发信人、发信人为电话号码
			   *  将BCD格式编码的电话号码转换为Unicode
			   */
			  unsigned char sender_len, unicode_buf[2],i = 0;
			  sender_len = buf[4]; // 单位：字符个数
			  while ( sender_len > 0 )
			  {
				  unicode_buf[1] = (buf[6+i]>>4) + '0';
				  unicode_buf[0] = 0x0;
				  rval = MessageBuildSender(buf[3]&0x3F, 2, unicode_buf);
				 
				  sender_len--;
				  if ( sender_len == 0 ) {
					  break;
				  }
				  
				  unicode_buf[1] = (buf[6+i] & 0xF) +'0';
				  unicode_buf[0] = 0x0;
				  
				  rval = MessageBuildSender(buf[3] & 0x3F, 2, unicode_buf);
				  sender_len--; i++;
			  }
			}	
		   
		   if ( buf[4] > 0 )
		   {
				NOTICE_ADD(NOTIFY_ID_APP_MSG );
				mq.type = NOTIFY_TYPE_WX;
		   }                       
		}else
		{
			/* 微信内容 */
			rval = MessageBuildContent(buf[3]&0x3F, buf[4], buf+6);
		}
		
		if ( rval != 0 )
		{
			txbuf[0] |= 0x1<<7;
		}		
		
	}  else if ( buf[2] == 0x5 )
	{
		/**
		 * Android APP (Whatsapp) 消息推送 */
		if ( buf[3] == 0x0 )
		{
			MessageClearAll(); /** 仅仅通知 */
			NOTICE_ADD(NOTIFY_ID_APP_MSG );
		}else
		if ( ((buf[3]>>7) & 0x1) == 0x1 ) 
		{
		   /* 发信人 */
		   MessageClearAll();
		   if ( (buf[3]>>6) & 0x1){ 
			   // 发信人为通讯录里面的名字
			   MessageBuildSender(buf[3]&0x3F,  buf[4], buf+6);
		   }else
		   {
			  /** 未知发信人、发信人为电话号码
			   *  将BCD格式编码的电话号码转换为Unicode
			   */
			  unsigned char sender_len, unicode_buf[2],i = 0;
			  sender_len = buf[4]; // 单位：字符个数
			  while ( sender_len > 0 )
			  {
				  unicode_buf[1] = (buf[6+i]>>4) + '0';
				  unicode_buf[0] = 0x0;
				  rval = MessageBuildSender(buf[3]&0x3F, 2, unicode_buf);
				 
				  sender_len--;
				  if ( sender_len == 0 ) {
					  break;
				  }
				  
				  unicode_buf[1] = (buf[6+i] & 0xF) +'0';
				  unicode_buf[0] = 0x0;
				  
				  rval = MessageBuildSender(buf[3] & 0x3F, 2, unicode_buf);
				  sender_len--; i++;
			  }
			}	
		   
		   if ( buf[4] > 0 )
		   {
				NOTICE_ADD(NOTIFY_ID_APP_MSG );
				mq.type = NOTIFY_TYPE_WHATSAPP;
			    mq.push_by_ios = 0;
		   }                       
		}else
		{
			/* WHATSAPP内容 */
			rval = MessageBuildContent(buf[3]&0x3F, buf[4], buf+6);
		}
		
		if ( rval != 0 )
		{
			txbuf[0] |= 0x1<<7;
		}
	} else if ( buf[2] == 0x6 )
	{
		/**
		 * Android APP (FaceBook) 消息推送 */
		if ( buf[3] == 0x0 )
		{
			MessageClearAll(); /** 仅仅通知 */
			NOTICE_ADD(NOTIFY_ID_APP_MSG );
		}else
		if ( ((buf[3]>>7) & 0x1) == 0x1 ) 
		{
		   /* 发信人 */
		   MessageClearAll();
		   if ( (buf[3]>>6) & 0x1){ 
			   // 发信人为通讯录里面的名字
			   MessageBuildSender(buf[3]&0x3F,  buf[4], buf+6);
		   }else
		   {
			  /** 未知发信人、发信人为电话号码
			   *  将BCD格式编码的电话号码转换为Unicode
			   */
			  unsigned char sender_len, unicode_buf[2],i = 0;
			  sender_len = buf[4]; // 单位：字符个数
			  while ( sender_len > 0 )
			  {
				  unicode_buf[1] = (buf[6+i]>>4) + '0';
				  unicode_buf[0] = 0x0;
				  rval = MessageBuildSender(buf[3]&0x3F, 2, unicode_buf);
				 
				  sender_len--;
				  if ( sender_len == 0 ) {
					  break;
				  }
				  
				  unicode_buf[1] = (buf[6+i] & 0xF) +'0';
				  unicode_buf[0] = 0x0;
				  
				  rval = MessageBuildSender(buf[3] & 0x3F, 2, unicode_buf);
				  sender_len--; i++;
			  }
			}	
		   
		   if ( buf[4] > 0 )
		   {
				NOTICE_ADD(NOTIFY_ID_APP_MSG );
				mq.type = NOTIFY_TYPE_FACEBOOK;
		   }                       
		}else
		{
			/* FACEBOOK内容 */
			rval = MessageBuildContent(buf[3]&0x3F, buf[4], buf+6);
		}
		
		if ( rval != 0 )
		{
			txbuf[0] |= 0x1<<7;
		}
	}	     // Line 通知
	else if ( buf[2] == 0x7 )
	{
		/**
		 * Android APP (Line) 消息推送 */
		if ( buf[3] == 0x0 )
		{
			MessageClearAll(); /** 仅仅通知 */
			NOTICE_ADD(NOTIFY_ID_APP_MSG );
			mq.type = NOTIFY_TYPE_LINE;
		}else
		if ( ((buf[3]>>7) & 0x1) == 0x1 ) 
		{
		   /* 发信人 */
		   MessageClearAll();
		   if ( (buf[3]>>6) & 0x1){ 
			   // 发信人为通讯录里面的名字
			   MessageBuildSender(buf[3]&0x3F,  buf[4], buf+6);
		   }else
		   {
			  /** 未知发信人、发信人为电话号码
			   *  将BCD格式编码的电话号码转换为Unicode
			   */
			  unsigned char sender_len, unicode_buf[2],i = 0;
			  sender_len = buf[4]; // 单位：字符个数
			  while ( sender_len > 0 )
			  {
				  unicode_buf[1] = (buf[6+i]>>4) + '0';
				  unicode_buf[0] = 0x0;
				  rval = MessageBuildSender(buf[3]&0x3F, 2, unicode_buf);
				 
				  sender_len--;
				  if ( sender_len == 0 ) {
					  break;
				  }
				  
				  unicode_buf[1] = (buf[6+i] & 0xF) +'0';
				  unicode_buf[0] = 0x0;
				  
				  rval = MessageBuildSender(buf[3] & 0x3F, 2, unicode_buf);
				  sender_len--; i++;
			  }
			}	
		   
		   if ( buf[4] > 0 )
		   {
				NOTICE_ADD(NOTIFY_ID_APP_MSG );
				mq.type = NOTIFY_TYPE_LINE;
                mq.push_by_ios = 0;
		   }                       
		}else
		{
			/* Line */
			rval = MessageBuildContent(buf[3]&0x3F, buf[4], buf+6);
		}
		
		if ( rval != 0 )
		{
			txbuf[0] |= 0x1<<7;
		}
	}else if ( buf[2] == 0x8 )
	{
		// twitter
		/**
		 * Android APP (Twitter) 消息推送 */
		if ( buf[3] == 0x0 )
		{
			MessageClearAll(); /** 仅仅通知 */
			NOTICE_ADD(NOTIFY_ID_APP_MSG );
			mq.type = NOTIFY_TYPE_TT;
		}else
		if ( ((buf[3]>>7) & 0x1) == 0x1 ) 
		{
		   /* 发信人 */
		   MessageClearAll();
		   if ( (buf[3]>>6) & 0x1){ 
			   // 发信人为通讯录里面的名字
			   MessageBuildSender(buf[3]&0x3F,  buf[4], buf+6);
		   }else
		   {
			  /** 未知发信人、发信人为电话号码
			   *  将BCD格式编码的电话号码转换为Unicode
			   */
			  unsigned char sender_len, unicode_buf[2],i = 0;
			  sender_len = buf[4]; // 单位：字符个数
			  while ( sender_len > 0 )
			  {
				  unicode_buf[1] = (buf[6+i]>>4) + '0';
				  unicode_buf[0] = 0x0;
				  rval = MessageBuildSender(buf[3]&0x3F, 2, unicode_buf);
				 
				  sender_len--;
				  if ( sender_len == 0 ) {
					  break;
				  }
				  
				  unicode_buf[1] = (buf[6+i] & 0xF) +'0';
				  unicode_buf[0] = 0x0;
				  
				  rval = MessageBuildSender(buf[3] & 0x3F, 2, unicode_buf);
				  sender_len--; i++;
			  }
			}	
		   
		   if ( buf[4] > 0 )
		   {
				NOTICE_ADD(NOTIFY_ID_APP_MSG );
				mq.type = NOTIFY_TYPE_TT;
		   }                       
		}else
		{
			/* Twitter */
			rval = MessageBuildContent(buf[3]&0x3F, buf[4], buf+6);
		}
		
		if ( rval != 0 )
		{
			txbuf[0] |= 0x1<<7;
		}		
	}else if ( buf[2] == 0x9 )
	{
		// skype
		/**
		 * Android APP (Skype) 消息推送 */
		if ( buf[3] == 0x0 )
		{
			MessageClearAll(); /** 仅仅通知 */
			NOTICE_ADD(NOTIFY_ID_APP_MSG );
			mq.type = NOTIFY_TYPE_SKYPE;
		}else
		if ( ((buf[3]>>7) & 0x1) == 0x1 ) 
		{
		   /* 发信人 */
		   MessageClearAll();
		   if ( (buf[3]>>6) & 0x1){ 
			   // 发信人为通讯录里面的名字
			   MessageBuildSender(buf[3]&0x3F,  buf[4], buf+6);
		   }else
		   {
			  /** 未知发信人、发信人为电话号码
			   *  将BCD格式编码的电话号码转换为Unicode
			   */
			  unsigned char sender_len, unicode_buf[2],i = 0;
			  sender_len = buf[4]; // 单位：字符个数
			  while ( sender_len > 0 )
			  {
				  unicode_buf[1] = (buf[6+i]>>4) + '0';
				  unicode_buf[0] = 0x0;
				  rval = MessageBuildSender(buf[3]&0x3F, 2, unicode_buf);
				 
				  sender_len--;
				  if ( sender_len == 0 ) {
					  break;
				  }
				  
				  unicode_buf[1] = (buf[6+i] & 0xF) +'0';
				  unicode_buf[0] = 0x0;
				  
				  rval = MessageBuildSender(buf[3] & 0x3F, 2, unicode_buf);
				  sender_len--; i++;
			  }
			}	
		   
		   if ( buf[4] > 0 )
		   {
				NOTICE_ADD(NOTIFY_ID_APP_MSG );
				mq.type = NOTIFY_TYPE_SKYPE;
		   }                       
		}else
		{
			/* Skype */
			rval = MessageBuildContent(buf[3]&0x3F, buf[4], buf+6);
		}
		
		if ( rval != 0 )
		{
			txbuf[0] |= 0x1<<7;
		}		
	}else if ( buf[2] == 10 )
	{
		// ins
		/**
		 * Android APP (INS) 消息推送 */
		if ( buf[3] == 0x0 )
		{
			MessageClearAll(); /** 仅仅通知 */
			NOTICE_ADD(NOTIFY_ID_APP_MSG );
			mq.type = NOTIFY_TYPE_INS;
		}else
		if ( ((buf[3]>>7) & 0x1) == 0x1 ) 
		{
		   /* 发信人 */
		   MessageClearAll();
		   if ( (buf[3]>>6) & 0x1){ 
			   // 发信人为通讯录里面的名字
			   MessageBuildSender(buf[3]&0x3F,  buf[4], buf+6);
		   }else
		   {
			  /** 未知发信人、发信人为电话号码
			   *  将BCD格式编码的电话号码转换为Unicode
			   */
			  unsigned char sender_len, unicode_buf[2],i = 0;
			  sender_len = buf[4]; // 单位：字符个数
			  while ( sender_len > 0 )
			  {
				  unicode_buf[1] = (buf[6+i]>>4) + '0';
				  unicode_buf[0] = 0x0;
				  rval = MessageBuildSender(buf[3]&0x3F, 2, unicode_buf);
				 
				  sender_len--;
				  if ( sender_len == 0 ) {
					  break;
				  }
				  
				  unicode_buf[1] = (buf[6+i] & 0xF) +'0';
				  unicode_buf[0] = 0x0;
				  
				  rval = MessageBuildSender(buf[3] & 0x3F, 2, unicode_buf);
				  sender_len--; i++;
			  }
			 }	
		   
		   if ( buf[4] > 0 )
		   {
				NOTICE_ADD(NOTIFY_ID_APP_MSG );
				mq.type = NOTIFY_TYPE_INS;
		   }                       
		}else
		{
			/* INS */
			rval = MessageBuildContent(buf[3]&0x3F, buf[4], buf+6);
		}
		
		if ( rval != 0 )
		{
			txbuf[0] |= 0x1<<7;
		}			
	}	
	else if ( buf[2] == 0x3){
		/**
		 * 电话挂断or接听,手环停止提醒
		 */
		// 去掉该功能
		notify_remove();
	}else{
		txbuf[0] |= 0x1<<7;
	}
	
	return 0;
}

void notify_remove(void)
{
	 win_ticks = 1000;
}
