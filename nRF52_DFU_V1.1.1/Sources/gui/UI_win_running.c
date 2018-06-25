
#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_GUI.h"
#include "Task_Step.h"
#include "Task_Store.h"
#include "LCD_ST7735.h"

#include "string.h"
#include "Window.h"
#include "UserSetting.h"
#include "UI_Draw.h"
#include "ClockTick.h"
#include "UI_hr_icon.h"

#include "UI_pedometer_icon.h"
#include "UI_win_running.h"
#include "UI_comm_icon.h"
#include "UI_Comm_Draw.h"
#include "QR_Utils.h"
#include "UI_main_icon.h"
#include "UI_function_icon.h"
#include "UI_bycle_icon.h"
#include "Ui_win_running.h"

unsigned char running_enable;

#define PW  4

void show_pedometer_val( int x,int y, int counter)
{
	int width = 0;
	int val = counter;
	
	do{
		width++;
		val /= 10;
	}while(val>0);
	
	val = counter;
	
	int sx = x+((80-x) - width*14)/2;
	
	int ex = sx + (width-1) * 14;
	do
	{
		draw_bitmap(ex, y, 16,14,Font16x14[val%10]);
		val /= 10; ex -= 14;
	}while( val > 0 );	
	
}

extern void show_Degree( int x, int y, int w, int h, int level );

static void show_pedometer(int x, int y )
{	
	#if 0
	draw_bitmap( 0,32,16,14,Font16x14[3]);
	draw_bitmap(16,32,16,14,Font16x14[6]);
	draw_bitmap(32,32,16,14,Font16x14[7]);
	draw_bitmap(48,32,16,14,Font16x14[8]);
	draw_bitmap(64,32,16,14,Font16x14[0]);
	#endif
	
	unsigned long temp;

	temp = pedometer.counter - config.train_running_pedometer;
	
	show_pedometer_val(0,20,temp);
	
	draw_bitmap( 54,70-8-20,23,9, gImage_steps_3);
	show_Degree( 0,86-8-20, 80, 1, 100);
		
	// 显示卡路里及图标
	draw_bitmap( 5, 80, 20, 20, gImage_bycle_calurie);
	
	temp = pedometer.calorie - config.train_running_calorie;
	draw_bitmap(  40, 86, 8, 12, Font_8x12_hr[(temp/100)%10]);	
	draw_bitmap(  48, 86, 8, 12, Font_8x12_hr[(temp/10)%10]);
	draw_bitmap(  56, 86, 8, 12, Font_8x12_hr[temp%10]);
	
	// 显示里程及图标
	draw_bitmap( 5, 112, 20, 20, gImage_bycle_distance);
	
	temp = pedometer.distance - config.train_running_distance;
	draw_bitmap(  40, 118, 8, 12, Font_8x12_hr[(temp/100)%10]);	
	draw_bitmap(  48, 118, 8, 12, Font_8x12_hr[(temp/10)%10]);
	draw_bitmap(  56, 118, 8, 12, Font_8x12_hr[temp%10]);	
	
	if ( running_enable == 1)
	{
		// 显示运动计时器
		
		unsigned long utc = getUTC() - config.train_running_utc;
		
		draw_bitmap(  8, 148, 8, 12, Font_8x12_hr[((utc/3600)/10)%10]);	
		draw_bitmap( 16, 148, 8, 12, Font_8x12_hr[(utc/3600)%10]);	
		draw_bitmap( 24, 148, 8, 12, Font_8x12_hr[10]);	
		draw_bitmap( 32, 148, 8, 12, Font_8x12_hr[((utc/60)/10)%10]);	
		draw_bitmap( 40, 148, 8, 12, Font_8x12_hr[(utc/60)%10]);	
		draw_bitmap( 48, 148, 8, 12, Font_8x12_hr[10]);	
		draw_bitmap( 56, 148, 8, 12, Font_8x12_hr[((utc%60)/10)%10]);	
		draw_bitmap( 64, 148, 8, 12, Font_8x12_hr[utc%10]);		
	}
}

unsigned long win_running_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0;  running_enable = 0;

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
		
		if ( win_ticks > 10 )
		{
			lcd_display(0);
			
			return ( msg_type ^ WINDOW_UPDATE );
		}
		
		memset( frame_buffer, 0, sizeof(frame_buffer));

		show_pedometer( 28, 20 );

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
			
			if ( running_enable == 0 )
			{
				msg_type |= WINDOW_DISTORY;				
			}
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
		} else {
			
			if ( running_enable == 0 )
			{
				running_enable = 1;
				
				config.train_running_utc  = getUTC();
				config.train_running_distance  = pedometer.distance;
				config.train_running_pedometer = pedometer.counter;
				config.train_running_calorie   = pedometer.calorie;
				
			} else
			{
				running_enable = 0;
				
				osal_set_event( taskStoreTaskId, TASK_STORE_SAVE_RUNNING_EVT );
			}
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
