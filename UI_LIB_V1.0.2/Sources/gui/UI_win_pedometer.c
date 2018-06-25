
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

#define COLOR_B      0X0000
#define COLOR_F      0xFFFF

void UI_Draw_Buttom(int index, int color);

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
		DrawUI_Bitmap_Ex(ex, y, 14,32,Font_14x32[val%10],0X97E0);
		val /= 10; ex -= 14;
	}while( val > 0 );	
}


#define PEDOMETER_COLOR_F 0XC7E0
#define PEDOMETER_COLOR_B 0x0000

extern unsigned char flag;

void show_pedometer(int x, int y )
{		
	draw_fill(0,0,80,160,COLOR_B);
	
	if ( flag++ & 0x1 )
	{
		DrawUI_Bitmap_Ex(4,8,72,72,BMP_pedometer_1, 0xFC20);
	}else{
		DrawUI_Bitmap_Ex(4,8,72,72,BMP_pedometer_2, 0xFC20);
	}
	show_pedometer_val((80-14*5)/2, 96, pedometer.counter);
	
	UI_Draw_Buttom(1,0xFC20);
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
			show_pedometer( 28, 20 );
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
		win_ticks = 0;
		
		if ( config.lcdEnable == 0 )
		{
			msg_type |= WINDOW_UPDATE;
		}else{
			
			if ( show_qr_flag == 1 && ble.isConnected == 1)
			{
				show_qr_flag = 0;				
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
			msg_type |= WINDOW_UPDATE;
			return ( msg_type ^ WINDOW_PRESS );
		}
		
		memset( frame_buffer, 0, sizeof(frame_buffer));
		if ( (show_qr_flag == 0) &&
			(ble.isConnected == 0) ){
			show_qr_flag = 1;
		}else{
			show_qr_flag = 0;
		}
		
		msg_type |= TASK_GUI_UPDATE_EVT;
		
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
