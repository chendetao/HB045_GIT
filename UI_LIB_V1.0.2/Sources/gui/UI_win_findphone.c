#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_GUI.h"
#include "LCD_ST7735.h"

#include "string.h"
#include "Window.h"
#include "UserSetting.h"
#include "UI_win_findphone.h"
#include "UI_Draw.h"

#include "UI_findphone_icon.h"
#include "Task_Finder.h"

#define COLOR_B      0X0000
#define COLOR_F      0X77E0

unsigned char showFlag;

extern unsigned char flag;

void UI_Draw_Buttom(int index, int color);

void show_findphone_icon(int x, int y )
{		
	static unsigned char showFlag = 0;
	
	if ( finder.isFound == 1 )
	{
		if ( flag++ & 0x1 )
		{
			DrawUI_Bitmap_Ex( 4,16,72,72,BMP_findphone_1, 0X77E0);
		}else{
			DrawUI_Bitmap_Ex( 4,16,72,72,BMP_findphone_2, 0X77E0);
		}
		
		UI_Draw_Buttom(6,0X77E0);
		
		return;
	}
	
	if ( finder.state == FIND_PHONE_STATE_STOP )
	{	
		if ( flag++ & 0x1 )
		{
			DrawUI_Bitmap_Ex( 4,16,72,72,BMP_findphone_1, 0X77E0);
		}else{
			DrawUI_Bitmap_Ex( 4,16,72,72,BMP_findphone_2, 0X77E0);
		}
		DrawUI_Bitmap_Ex(16,116,48,16,bmp_str_finding, 0xFFFF);	
		
		UI_Draw_Buttom(6, 0X77E0);
	}else{                  
		
		showFlag = !showFlag;
		if ( showFlag & 0x1 ) {
			DrawUI_Bitmap_Ex( 4,16,72,72,BMP_finding_1, 0X77E0);	
		}else{
			DrawUI_Bitmap_Ex( 12,16,56,56,BMP_finding_2, 0X77E0);
		}
		                                           // bmp_str_find
		DrawUI_Bitmap_Ex(16,116,48,16,bmp_str_finding, 0xFFFF);
	}
}

unsigned long win_findphone_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0; showFlag = 0; 
		finder.state = FIND_PHONE_STATE_STOP;
		
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
		
        /* 当前不处于查找状态中，2.5秒自动灭屏 */
        if ( (win_ticks == 6) && (finder.state != FIND_PHONE_STATE_FINDING ))
        {
            lcd_display(0); 
            finder.isFound = 0;
			
			return ( msg_type ^ WINDOW_UPDATE );
        }
        
        /* 11秒后自动停止查找 */
        if ( (win_ticks > 22) ) 
        {
			lcd_display(0); 
			
			finder.isFound = 0;
			
			if (finder.state == FIND_PHONE_STATE_FINDING )
			{
				osal_set_event ( taskFinderTaskId, FIND_HONE_TASK_STOP_EVT );
			}
			
            finder.state = FIND_PHONE_STATE_STOP;			
			
			return ( msg_type ^ WINDOW_UPDATE );
        }		
				
		draw_fill(0,0,80,160,COLOR_B);
		show_findphone_icon( 28, 20 );
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
			// add 2018.03.26
			osal_set_event ( taskFinderTaskId, FIND_HONE_TASK_STOP_EVT );
			window_push(window_get(win_current));
			msg_type |= WINDOW_DISTORY;		
		}
		
		return ( msg_type ^ WINDOW_TOUCH );
	}
	
	if ( msg_type & WINDOW_PRESS )
	{		
		/***********************************************
		 *                                             *
		 * 长按开始查找/取消查找手机                   *
		 *                                             *
		 **********************************************/                                             
		 
		win_ticks = 0;
		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);			
			msg_type |= WINDOW_UPDATE;

			return ( msg_type ^ WINDOW_PRESS );
		}
		
		if ( finder.state == FIND_PHONE_STATE_STOP )
		{
			finder.state = FIND_PHONE_STATE_FINDING;
			showFlag = 0;  
			finder.isFound = 0;
			osal_set_event ( taskFinderTaskId, FIND_HONE_TASK_START_EVT );
		}else
		{
			finder.state = FIND_PHONE_STATE_STOP;
			osal_set_event ( taskFinderTaskId, FIND_HONE_TASK_STOP_EVT );
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
		finder.isFound = 0;
        // 退出该界面时，发现状态更改为 0，以便下次可进入发现在状态.
        finder.state = FIND_PHONE_STATE_STOP;		
		
		window_pop(&win_current);
		
		return window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0 );
	}	
	
	return 0;
}
