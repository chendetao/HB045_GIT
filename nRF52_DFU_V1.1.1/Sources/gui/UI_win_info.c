
#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_GUI.h"
#include "Task_Step.h"
#include "LCD_ST7735.h"

#include "string.h"
#include "Window.h"
#include "UserSetting.h"
#include "UI_Draw.h"

#include "UI_info_icon.h"
#include "UI_comm_icon.h"
#include "UI_Comm_Draw.h"
#include "UI_main_icon.h"
#include "UI_win_info.h"
#include "UI_alarm_icon.h"
#include "HFMA2Ylib.h"
#include "version.h"
#include "SPI.h"

extern unsigned char mac_buf[18];
extern unsigned char DZ[32];
extern void DoRect8x16( int cx, int cy, const unsigned char pic[32]);

void show_information_val(int x, int y, int distance )
{
	// "MAC"
	ASCII_GetData('M',ASCII_8X16, DZ);
	DoRect8x16(28, 24, DZ);
	ASCII_GetData('A',ASCII_8X16, DZ);
	DoRect8x16(36, 24, DZ);
	ASCII_GetData('C',ASCII_8X16, DZ);
	DoRect8x16(44, 24, DZ);	
	
	// address
	ASCII_GetData(mac_buf[0],ASCII_8X16, DZ);
	DoRect8x16( 8, 64, DZ);
	ASCII_GetData(mac_buf[1],ASCII_8X16, DZ);
	DoRect8x16(16, 64, DZ);
	ASCII_GetData(mac_buf[2],ASCII_8X16, DZ);
	DoRect8x16(24, 64, DZ);
	ASCII_GetData(mac_buf[3],ASCII_8X16, DZ);
	DoRect8x16(32, 64, DZ);
	ASCII_GetData(mac_buf[4],ASCII_8X16, DZ);
	DoRect8x16(40, 64, DZ);
	ASCII_GetData(mac_buf[5],ASCII_8X16, DZ);
	DoRect8x16(48, 64, DZ);
	ASCII_GetData(mac_buf[6],ASCII_8X16, DZ);
	DoRect8x16(56, 64, DZ);
	ASCII_GetData(mac_buf[7],ASCII_8X16, DZ);
	DoRect8x16(64, 64, DZ);
	
	ASCII_GetData(mac_buf[9],ASCII_8X16, DZ);
	DoRect8x16( 8, 80, DZ);
	
	ASCII_GetData(mac_buf[10],ASCII_8X16, DZ);
	DoRect8x16(16, 80, DZ);
	ASCII_GetData(mac_buf[11],ASCII_8X16, DZ);
	DoRect8x16(24, 80, DZ);
	ASCII_GetData(mac_buf[12],ASCII_8X16, DZ);
	DoRect8x16(32, 80, DZ);
	ASCII_GetData(mac_buf[13],ASCII_8X16, DZ);
	DoRect8x16(40, 80, DZ);
	ASCII_GetData(mac_buf[14],ASCII_8X16, DZ);
	DoRect8x16(48, 80, DZ);
	ASCII_GetData(mac_buf[15],ASCII_8X16, DZ);
	DoRect8x16(56, 80, DZ);
	ASCII_GetData(mac_buf[16],ASCII_8X16, DZ);
	DoRect8x16(64, 80, DZ);		
}

void ui_show_version(void)
{  
    ASCII_GetData('V',ASCII_8X16,DZ); // Vx.x.x
    DoRect8x16(16, 120, DZ);              
    ASCII_GetData('0'+VERSION_MAJOR,ASCII_8X16,DZ);
    DoRect8x16(24, 120, DZ);
    ASCII_GetData('.',ASCII_8X16,DZ);
    DoRect8x16(32, 120, DZ);    
    ASCII_GetData('0'+VERSION_MINOR,ASCII_8X16,DZ);
    DoRect8x16(40, 120, DZ);
    ASCII_GetData('.',ASCII_8X16,DZ);
    DoRect8x16(48, 120, DZ);    
    ASCII_GetData('0'+VERSION_REVISON,ASCII_8X16,DZ);
    DoRect8x16(56, 120, DZ);      
}

void show_information(int x, int y, int counter )
{		
	ExitDeepSleepMode();
	
	show_information_val( 56, 29, pedometer.distance );
	ui_show_version();
	
	EntryDeepSleepMode();
}

unsigned long win_info_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0;

		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
		}
		
		osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
		
		return ( msg_type ^ WINDOW_CREATE );
	}
	
	if ( msg_type & WINDOW_UPDATE )
	{
		win_ticks++;
		
		if ( win_ticks > 5 )
		{
			lcd_display(0);
			
			return ( msg_type ^ WINDOW_UPDATE );
		}
           		
		memset(frame_buffer, 0x00, sizeof(frame_buffer));
		show_information( 28, 20, pedometer.distance);
		sync_frame();
		
		osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 500);
		
		return ( msg_type ^ WINDOW_UPDATE );
	}
	
	if ( msg_type & WINDOW_TOUCH )
	{		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);			
			win_ticks = 0;
			msg_type |= WINDOW_UPDATE;
		}else{
			msg_type |= WINDOW_DISTORY;		
		}
		
		return ( msg_type ^ WINDOW_TOUCH );
	}
	
	if ( msg_type & WINDOW_PRESS )
	{
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);			
			win_ticks = 0;
			msg_type |= WINDOW_UPDATE;
		}else{
			msg_type |= WINDOW_DISTORY;		
		}
		
		return ( msg_type ^ WINDOW_PRESS );
	}
	
	if ( msg_type & WINDOW_NOTIFY )
	{   
        /* 当前窗体入栈 */
        window_push( win_current );
        
        /* 通知界面作为当前界面 */
        win_current = NOTIFY_WIN_ID_GET();
        
        /* 调用窗体回调函数以便创建窗体  */
        window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0);
		
		return ( msg_type ^ WINDOW_NOTIFY );
	}	
	
	if ( msg_type & WINDOW_DISTORY )
	{
		window_pop(&win_current);
		
		return window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0 );
	}	
	
	return 0;
}
