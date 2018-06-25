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
#include "UI_sport_icon.h"

#define COLOR_B      0X0000
#define COLOR_F      0XF3E0

extern unsigned char flag;

void UI_Draw_Buttom(int index, int color);

void show_alarm(int x, int y, int hour, int minute )
{	
	draw_fill(0,0,80,160,COLOR_B);
	
	if ( flag++ & 0x1 )
	{
		DrawUI_Bitmap_Ex( 4,16,72,72,BMP_alarm_1, 0XFB24);
		DrawUI_Bitmap_Ex( 4,16,72,72,BMP_alarm_h, 0xFFFF);
	}else{
		DrawUI_Bitmap_Ex( 4,16,72,72,BMP_alarm_2, 0XFB24);
		DrawUI_Bitmap_Ex( 4,16,72,72,BMP_alarm_h, 0xFFFF);		
	}
	
	DrawUI_Bitmap( 0+2+2+8, 112, 11,24,Font_11x24[hour/10],0xFFFF,COLOR_B);
	DrawUI_Bitmap(12+2+2+8, 112, 11,24,Font_11x24[hour%10],0xFFFF,COLOR_B);
	DrawUI_Bitmap(24+2-1+2+8, 112, 11,24,Font_11x24[11],0xFFFF,COLOR_B);
	DrawUI_Bitmap(36+2-3+2+8, 112, 11,24,Font_11x24[minute/10],0xFFFF,COLOR_B);
	DrawUI_Bitmap(48+2-3+2+8, 112, 11,24,Font_11x24[minute%10],0xFFFF,COLOR_B);	
	
	UI_Draw_Buttom(5,0XFB24);
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
			window_push(window_get(win_current));
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
