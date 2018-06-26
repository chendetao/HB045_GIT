#include "string.h"
#include "Window.h"

#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_GUI.h"

#include "LCD_ST7735.h"
#include "QR_Utils.h"
#include "Alarm.h"

#include "UI_win_hr.h"
#include "UserSetting.h"
#include "UI_Draw.h"

#include "UI_win_alarm.h"
#include "UI_alarm_icon.h"
#include "UI_comm_icon.h"
#include "UI_function_icon.h"

#define COLOR_F 0xFFFF
#define COLOR_B 0x0000

void show_alarm(int x, int y, int hour, int minute )
{	
    #if 0
	DrawUI_Bitmap(0,16,80,80,BMP_function_alarm,BMP_FUNCTION_ALARM_RGB_F,BMP_FUNCTION_ALARM_RGB_B);	
	#else
    draw_bitmap_from_flash( (80-64)/2, 16, 64, 64, 0x200000+0x6000);    
    #endif
    
	DrawUI_Bitmap(13,114,11,24,Font_11x24[hour/10], COLOR_F,COLOR_B);	
	DrawUI_Bitmap(24,114,11,24,Font_11x24[hour%10], COLOR_F,COLOR_B);	
	DrawUI_Bitmap(35,114,11,24,Font_11x24[11], COLOR_F,COLOR_B);	
	DrawUI_Bitmap(45,114,11,24,Font_11x24[minute/10], COLOR_F,COLOR_B);	
	DrawUI_Bitmap(56,114,11,24,Font_11x24[minute%10], COLOR_F,COLOR_B);	
}

unsigned long win_alarm_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{	
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0;

		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
		}
		
		show_alarm(0,0,alarm.g_Ahour_next, alarm.g_Amin_next);
		
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
		
		memset(frame_buffer, 0, sizeof(frame_buffer));				
		show_alarm(0,0,alarm.g_Ahour_next, alarm.g_Amin_next);
		sync_frame();
		
		osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 500);
		
		return ( msg_type ^ WINDOW_UPDATE );
	}
	
	if ( msg_type & WINDOW_TOUCH )
	{	
		if ( config.lcdEnable == 0)
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
		if ( config.lcdEnable == 0)
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
		window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0 );
		
		return ( msg_type ^ WINDOW_DISTORY );
	}

    return 0;
}
