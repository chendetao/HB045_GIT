
#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_GUI.h"
#include "Task_Step.h"
#include "LCD_ST7735.h"

#include "string.h"
#include "Window.h"
#include "UserSetting.h"
#include "UI_Draw.h"

#include "UI_pedometer_icon.h"
#include "UI_comm_icon.h"
#include "UI_Comm_Draw.h"
#include "QR_Utils.h"
#include "UI_main_icon.h"
#include "UI_function_icon.h"

void show_Degree( int x, int y, int w, int h, int level )
{
	for ( int j = y; j < y + h; j++ )
	{
		for ( int i = x; i < x + w; i++ )
		{
			if ( !(i%2) )
			{
				set_pixel(i, j, 0xFFFF);
			}
		}
	}
	
	for ( int j = y; j < y + h; j++ )
	{
		for ( int i = x; i < x + w; i++ )
		{
			if ( !(i%2) )
			{
				set_pixel(i, j, 0xFFFF);
			}
		}
	}	
}

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

#define COLOR_F 0xFFFF
#define COLOR_B 0x0000

void show_distance_val( int x, int y, int distance )
{
	float value;
	
	if ( config.unit == 0 )
	{
		value = distance / 1000.0 + 0.05;
	}else{
		value = distance * 0.0006214 + 0.05;
	}	
	
	int vh = (int)value;
	int vl = (int)((value - vh)*10);
	
	if ( vh > 99 ){
		vh = 99;
	}
	
	if ( vh > 9 )
	{
		DrawUI_Bitmap( x, y, 7, 16,Font_7x16[vh/10], COLOR_F,COLOR_B);
	}
	DrawUI_Bitmap(x+7, y, 7, 16,Font_7x16[vh%10], COLOR_F,COLOR_B);
	DrawUI_Bitmap(x+14,y, 7, 16,Font_7x16[10], COLOR_F,COLOR_B);	
	DrawUI_Bitmap(x+14+5,y, 7, 16,Font_7x16[vl], COLOR_F,COLOR_B);	
}

void show_calorie_val( int x, int y, int calorie )
{
	int val =  calorie;
	int count = 0, width = 0;
	
	do{	
		count++;
		val /= 10;
	}while(val>0);
	
	width = x + (61- x - count*7)/2;
	x = width + (count-1)*7;
	val = calorie;
	do{
		DrawUI_Bitmap(x, y, 7, 16,Font_7x16[val%10], COLOR_F,COLOR_B);
		val /= 10;
		x -= 7;
	}while(val > 0 );
}

extern void DrawBackGroud(void);

void show_pedometer(int x, int y, int counter )
{	
    DrawBackGroud();	
	
	show_pedometer_val(0,24, counter);
	
	draw_bitmap( 0,56-8,26,28, gImage_running);
	DrawUI_Bitmap(48,61,32,16,bmp_steps_pedometer, 0x07EC,COLOR_B);
	
	draw_bitmap( 0, 90, 26, 26, gImage_distance);
	draw_bitmap( 0, 126, 24, 24, gImage_calurie_ss);
	
	// show distance and calorie 

	if ( config.unit == 0 )
	{
		show_distance_val(31, 96, pedometer.distance);
	}else
	{
		show_distance_val(28, 96, pedometer.distance);
	}
	show_calorie_val(22, 131, pedometer.calorie);
	
	if ( config.unit == 0 )
	{	
		DrawUI_Bitmap(62,96,32,16,bmp_km_pedometer, COLOR_F,COLOR_B);
	} else {	
		DrawUI_Bitmap(62-5,96,32,16,bmp_mile_pedometer, COLOR_F,COLOR_B);
	}		
	
	DrawUI_Bitmap(62-6,131,32,16,bmp_kc_pedometer, COLOR_F,COLOR_B);
	
	show_Degree( 0,86-8, 80, 1, 100);
}

void show_qr(void)
{
	QR_Copy_Image(frame_buffer);
}

unsigned long show_qr_flag = 0;
unsigned long win_pedometer_proc(unsigned long wid, unsigned long msg_type, 
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
		
		if (( (win_ticks > 5) && (show_qr_flag == 0) )
			|| ( win_ticks > 60 ) )
		{
			lcd_display(0);
			
			return ( msg_type ^ WINDOW_UPDATE );
		}
				
		memset( frame_buffer, 0, sizeof(frame_buffer));
		
		if ( (show_qr_flag == 1) && (ble.isConnected == 0))
		{ 
			show_qr();
		}else{
			show_pedometer( 28, 20, pedometer.counter);
            if ( show_qr_flag == 1 )
            {
                show_qr_flag = 0;
            }
		}        
		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
		}		
		sync_frame();
		
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
			
			if ( show_qr_flag == 1 && ble.isConnected == 1)
			{
				win_ticks = 0;
				show_qr_flag = 0;
				msg_type |= WINDOW_UPDATE;
				
				return ( msg_type ^ WINDOW_TOUCH );
			}
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
		if ( (show_qr_flag == 0) &&
			(ble.isConnected == 0) ){
			show_qr();
			show_qr_flag = 1;
		}else{
			show_qr_flag = 0;
		}
		
		msg_type |= WINDOW_UPDATE;
		
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
		show_qr_flag = 0;		
		window_pop(&win_current);
		
		return window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0 );
	}	
	
	return 0;
}
