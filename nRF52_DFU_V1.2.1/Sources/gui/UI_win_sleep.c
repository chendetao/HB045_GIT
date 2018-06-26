#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_GUI.h"
#include "Task_Step.h"
#include "LCD_ST7735.h"

#include "string.h"
#include "Window.h"
#include "UserSetting.h"
#include "UI_Draw.h"

#include "UI_sleep_icon.h"
#include "UI_comm_icon.h"
#include "UI_function_icon.h"
#include "sleep_algorithm.h"

#define COLOR_F 0xffff
#define COLOR_B 0x0000

void show_sleep_timer(int x, int y, int deep_time,int light_time)
{
    float val_deep = deep_time/60.0f + 0.0500001f;
    float val_light = light_time/60.0f + 0.0500001f;
    
    int dh = (int)val_deep;
    int dl = (int)((val_deep-dh)*10);
    int lh = (int)val_light;
    int ll = (int)((val_light-lh)*10);
    
    int vh = dh+lh;
    int vl = dl + ll;
    if ( vl > 9 )
    {
      vl -= 10;
      vh ++;
    }
	/** */
	
	DrawUI_Bitmap(0,16,80,80,BMP_function_sleep,0XF81F,BMP_FUNCTION_SLEEP_RGB_B);	
	
	if ( (int)(vh) > 9 )
	{
		x = 0 + ((80-0)-3*11-11)/2;
		DrawUI_Bitmap( x, 112, 11,24,Font_11x24[(vh/10)%10], COLOR_F,COLOR_B);
		x += 8;
	}else{
		x = 0 + ((80-0)-2*11-3)/2;
	}	
	DrawUI_Bitmap( x, 112, 11,24, Font_11x24[vh%10], COLOR_F,COLOR_B);
	x += 11;
	DrawUI_Bitmap( x, 112, 11,24, Font_11x24[10], COLOR_F,COLOR_B);
	x += 11;
	DrawUI_Bitmap( x+1, 112, 11,24, Font_11x24[vl%10], COLOR_F,COLOR_B);		
}

unsigned long win_sleep_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0;		
		
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
		show_sleep_timer(0,0, AutoSleep.out_TotalDeepTime, AutoSleep.out_TotalLightTime);			
		sync_frame();
		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
		}		
		
		osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 500);
		
		return ( msg_type ^ WINDOW_UPDATE );
	}
	
	if ( msg_type & WINDOW_TOUCH )
	{		
		if ( config.lcdEnable == 0 )
		{
			win_ticks = 0;
			msg_type |= WINDOW_UPDATE;
		}else{
			// window_push(window_get(win_current));
			msg_type |= WINDOW_DISTORY;		
		}
		
		return ( msg_type ^ WINDOW_TOUCH );
	}
	
	if ( msg_type & WINDOW_PRESS )
	{		
		win_ticks = 0;
		
		if ( config.lcdEnable == 0 )
		{
			msg_type |= WINDOW_UPDATE;
			
			return ( msg_type ^ WINDOW_PRESS );
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
