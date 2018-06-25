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

#include "sleep_algorithm.h"
#include "UI_comm_icon.h"

#define COLOR_B      0X0000
#define COLOR_F      0xFFFF
#define SLEEP_COLOR_B 0x0000
#define SLEEP_COLOR_F 0xD01F

static unsigned char flag;

void UI_Draw_Buttom(int index, int color);

void DrawSleepTimer( int x, int y, int deep_time, int light_time )
{
	draw_fill(0,0,LCD_W,LCD_H,COLOR_B);
	
	if ( flag++ & 0x1 )
	{
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_sleep_1, 0XA81F);	
	} else {
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_sleep_2, 0XA81F);	
	}
	
	#if 0
	
	DrawUI_Bitmap(48, 98,32,16, bmp_str_sleep, COLOR_F,COLOR_B);
	DrawUI_Bitmap(48,114,30,16, bmp_str_deep, COLOR_F,COLOR_B);
	DrawUI_Bitmap(48,130,28,16, bmp_str_light,COLOR_F,COLOR_B);
	
	// 统计
	int h = (int)((AutoSleep.out_TotalSleepTime/60.0f) + 0.5f);
	int l = (int)(((AutoSleep.out_TotalSleepTime - h)/60.0f)+0.5f) * 10;
	
	DrawUI_Bitmap( 0+4, 96, 8, 16, Font_8x16_Other[(h/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 8+4, 96, 8, 16, Font_8x16_Other[h%10],COLOR_F,COLOR_B);	
	DrawUI_Bitmap(16+4, 96, 8, 16, Font_8x16_Other[10],COLOR_F,COLOR_B);	
	DrawUI_Bitmap(24+4, 96, 8, 16, Font_8x16_Other[l%10],COLOR_F,COLOR_B);	
	
	DrawUI_Bitmap(32+4, 96, 8, 16, bmp_str_unit_h, COLOR_F,COLOR_B); 
		
	// 深睡
	h = (int)((AutoSleep.out_TotalDeepTime/60.0f) + 0.5f);
	l = (int)(((AutoSleep.out_TotalDeepTime - h)/60.0f)+0.5f) * 10;	
	
	DrawUI_Bitmap( 0+4, 112, 8, 16, Font_8x16_Other[(h/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 8+4, 112, 8, 16, Font_8x16_Other[h%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(16+4, 112, 8, 16, Font_8x16_Other[10],COLOR_F,COLOR_B);	
	DrawUI_Bitmap(24+4, 112, 8, 16, Font_8x16_Other[l%10],COLOR_F,COLOR_B);

	DrawUI_Bitmap(32+4, 112, 8, 16, bmp_str_unit_h, COLOR_F,COLOR_B); 
	
	// 浅睡
	h = (int)((AutoSleep.out_TotalLightTime/60.0f) + 0.5f);
	l = (int)(((AutoSleep.out_TotalLightTime - h)/60.0f)+0.5f) * 10;	
	
	DrawUI_Bitmap( 0+4, 128, 8, 16, Font_8x16_Other[(h/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 8+4, 128, 8, 16, Font_8x16_Other[h%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(16+4, 128, 8, 16, Font_8x16_Other[10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(24+4, 128, 8, 16, Font_8x16_Other[l%10],COLOR_F,COLOR_B);

	DrawUI_Bitmap(32+4, 128, 8, 16, bmp_str_unit_h, COLOR_F,COLOR_B); 
	#else
	
    float val_deep = deep_time/60.0f + 0.050001f;
    float val_light = light_time/60.0f + 0.050001f;
    
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
	
	x = 12;
	if ( vh > 9 )
	{
		DrawUI_Bitmap( x, 112, 11,24,Font_11x24[vh/10],COLOR_F,COLOR_B);
		x += 12;
	} else {
		x += 8;
	}

	DrawUI_Bitmap(   x, 112, 11,24,Font_11x24[vh%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(x+12, 112, 11,24,Font_11x24[12],COLOR_F,COLOR_B);
	DrawUI_Bitmap(x+12+6, 112, 11,24,Font_11x24[vl],COLOR_F,COLOR_B);
	DrawUI_Bitmap(x+12+5+12+4, 112, 11,24,Font_11x24[10],0XA81F,COLOR_B);
	
	#endif
	
	UI_Draw_Buttom(4,0XA81F);
}

unsigned long win_sleep_proc(unsigned long wid, unsigned long msg_type, 
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
					
		DrawSleepTimer(0,0,AutoSleep.out_TotalDeepTime, AutoSleep.out_TotalLightTime );
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
			window_push(window_get(win_current));
			msg_type |= WINDOW_DISTORY;		
		}
		
		return ( msg_type ^ WINDOW_TOUCH );
	}
	
	if ( msg_type & WINDOW_PRESS )
	{		
		win_ticks = 0;
		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
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
