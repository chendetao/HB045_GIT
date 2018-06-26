
#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_GUI.h"
#include "Task_Step.h"
#include "LCD_ST7735.h"

#include "string.h"
#include "Window.h"
#include "UserSetting.h"
#include "UI_Draw.h"

#include "UI_function_icon.h"
#include "UI_win_function.h"
#include "UI_comm_icon.h"
#include "UI_Comm_Draw.h"

int selected = 0;

int window_cnt = 0;

extern void DrawBackGroud(void);

#include "UI_Flash_Lib.h"

void show_function(int x, int y, int counter )
{		
    if ( selected == 0 )                  		// 骑行
    {
        DrawUI_Bitmap((80-32)/2,8-4,32,32,bmp_function_back,0X0946,BMP_FUNCTION_BACK_RGB_B);	
        // DrawUI_Bitmap(0,40,80,80,BMP_function_bycle,BMP_FUNCTION_BYCLE_RGB_F,BMP_FUNCTION_BYCLE_RGB_B);	
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x0000);
        // 双色
        DrawUI_Bitmap((80-32)/2,120,32,32,bmp_hr_1, 0X3000,0x00); 
        DrawUI_Bitmap_Ex((80-32)/2,120+4,32,32,bmp_hr_2, 0X3000);
    }else if ( selected == 1 )                 	// 心率
    {	
        DrawUI_Bitmap((80-32)/2,8-4,32,32,bmp_function_bycle,0X30C1,BMP_FUNCTION_BYCLE_RGB_B);		
        
        #if 0
        // 双色
        DrawUI_Bitmap(0,40,80,80,BMP_hr_1, 0x1F<<11,0x00); 
        DrawUI_Bitmap_Ex(0,40,80,80,BMP_hr_2, 0xFFFF);	
        #else
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x2000);
        #endif
        
        DrawUI_Bitmap((80-32)/2,120+4,32,32,bmp_function_sleep,0x800F,BMP_FUNCTION_SLEEP_RGB_B);	
    }else if ( selected == 2 )                  // 睡眠
    {
        // 双色
        DrawUI_Bitmap((80-32)/2,8-4,32,32,bmp_hr_1, 0X3000,0x00); 
        DrawUI_Bitmap_Ex((80-32)/2,8,32,32,bmp_hr_2, 0X3000);
        #if 0
        DrawUI_Bitmap(0,40,80,80,BMP_function_sleep,0XF81F,BMP_FUNCTION_SLEEP_RGB_B);
        #else
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x4000);
        #endif
        DrawUI_Bitmap((80-32)/2,120+4,32,32,bmp_function_alarm,0X5162,BMP_FUNCTION_ALARM_RGB_B);		
    } else if ( selected == 3 )					// 闹钟
    {
        DrawUI_Bitmap((80-32)/2,8-4,32,32,bmp_function_sleep,0x800F,BMP_FUNCTION_SLEEP_RGB_B);	
        #if 0
        DrawUI_Bitmap(0,40,80,80,BMP_function_alarm,BMP_FUNCTION_ALARM_RGB_F,BMP_FUNCTION_ALARM_RGB_B);
        #else
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x6000);
        #endif        
        DrawUI_Bitmap((80-32)/2+3,120+4,32,32,bmp_function_findphone,0X1983,BMP_FUNCTION_FINDPHONE_RGB_B);			
    } else if ( selected == 4 )               	// 手机
    {
        DrawUI_Bitmap((80-32)/2,8-4,32,32,bmp_function_alarm,0X5162,BMP_FUNCTION_ALARM_RGB_B);	
        #if 0
        DrawUI_Bitmap(0,40,80,80,BMP_function_findphone,BMP_FUNCTION_FINDPHONE_RGB_F,BMP_FUNCTION_FINDPHONE_RGB_B);
        #else
        draw_bitmap_from_flash( (80-64)/2+8, (160-64)/2, 64, 64, 0x200000+0x8000);
        #endif        
        DrawUI_Bitmap((80-32)/2,120+4,32,32,bmp_function_info,0X30E1,BMP_FUNCTION_INFO_RGB_B);		
    } else if ( selected == 5 )                 // 信息
    {
        DrawUI_Bitmap((80-32)/2+3,8-4,32,32,bmp_function_findphone,0X1983,BMP_FUNCTION_FINDPHONE_RGB_B);
        #if 0
        DrawUI_Bitmap(0,40,80,80,BMP_function_info,BMP_FUNCTION_INFO_RGB_F,BMP_FUNCTION_INFO_RGB_B);
        #else
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0xa000);
        #endif        
        DrawUI_Bitmap((80-32)/2,120+4,32,32,bmp_function_shutdown,0X3061,BMP_FUNCTION_SHUTDOWN_RGB_B);	
    } else if ( selected == 6 )					// 关机
    {
        DrawUI_Bitmap((80-32)/2,8-4,32,32,bmp_function_info,0X30E1,BMP_FUNCTION_INFO_RGB_B);
        #if 0
        DrawUI_Bitmap(0,40,80,80,BMP_function_shutdown,BMP_FUNCTION_SHUTDOWN_RGB_F,BMP_FUNCTION_SHUTDOWN_RGB_B);
        #else
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0xc000);
        #endif        
        DrawUI_Bitmap((80-32)/2,120+4,32,32,bmp_function_back,0X0946,BMP_FUNCTION_BACK_RGB_B);
    }else if ( selected == 7 )					// 返回
    {
        DrawUI_Bitmap((80-32)/2,8-4,32,32,bmp_function_shutdown,0X3061,BMP_FUNCTION_SHUTDOWN_RGB_B);
        #if 0
        DrawUI_Bitmap(0,40,80,80,BMP_function_back,BMP_FUNCTION_BACK_RGB_F,BMP_FUNCTION_BACK_RGB_B); 
        #else
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0xe000);
        #endif        
        DrawUI_Bitmap((80-32)/2,120+4,32,32,bmp_function_bycle,0X30C1,BMP_FUNCTION_BYCLE_RGB_B);		
    }else{
        selected = 0;
    }
}

unsigned long win_function_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0; // selected = 0;

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

		/**
		 *  增加以下语句,2秒内自动跳转选中的功能界面
		 */
		// if ( win_ticks > 3 )
		// {
		//	msg_type |= WINDOW_SWITCH;
		//	return ( msg_type ^ WINDOW_UPDATE );
		// }		
		
		// memset(frame_buffer, 0x00, sizeof(frame_buffer));
        
		DrawBackGroud();
		show_function( 28, 20, pedometer.calorie);
		sync_frame();
		
		osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 500);
		
		return ( msg_type ^ WINDOW_UPDATE );
	}
	
	if ( msg_type & WINDOW_TOUCH )
	{		
		win_ticks = 0;
		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);			
		}else{	

			if ( selected == 7 )
			{
				window_push(WIN_ID_TIMER); 	 // back window
				msg_type |= WINDOW_DISTORY;
				return ( msg_type ^ WINDOW_TOUCH );
			} else {
				selected++; selected %= 8;
			}
		}
		
		msg_type |= WINDOW_UPDATE;
		
		return ( msg_type ^ WINDOW_TOUCH );
	}
	
	if ( msg_type & WINDOW_SWITCH )
	{
		win_ticks = 0;
		
		window_push(win_current);	
		
		switch ( selected )
		{
		case 0:	
			window_push(WIN_ID_SPORT); 	 // bycle window
			break;
		case 1:
			window_push(WIN_ID_HR); 	 // hr window
			break;
		case 2:
			window_push(WIN_ID_SLEEP); 	 // sleep window
			break;
		case 3:
			window_push(WIN_ID_ALARM); 	 // alarm window
			break;
		case 4:
			window_push(WIN_ID_FIND); 	 // findphone window
			break;
		case 5:
			window_push(WIN_ID_INFO);  	 // info window
			break;
		case 6:
			window_push(WIN_ID_SHUTDOWN);// shutdown window
			break;	
		case 7:
			window_push(WIN_ID_TIMER); 	 // back window
			break;
		default:
			break;
		}
		
		msg_type |= WINDOW_DISTORY;
		
		// lcd_display(0);
			
		return ( msg_type ^ WINDOW_SWITCH );
	}
	
	if ( msg_type & WINDOW_PRESS )
	{		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
			msg_type |= WINDOW_UPDATE;
		}else{
			msg_type |= WINDOW_SWITCH;
		}
		
		win_ticks = 0;
		
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
