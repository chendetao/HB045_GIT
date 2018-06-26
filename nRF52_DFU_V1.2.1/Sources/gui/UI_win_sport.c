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
#include "ClockTick.h"

#include "UI_comm_icon.h"
#include "UI_Comm_Draw.h"
#include "QR_Utils.h"
#include "UI_main_icon.h"
#include "Ui_win_running.h"
#include "Task_Flash.h"
#include "UI_sport_icon.h"
#include "UI_win_sport.h"
#include "UI_main_icon.h"
#include "UI_function_icon.h"
#include "Task_Touch.h"

#define SPORT_WIN_ID_NR      4

unsigned long sport_id, sport_stat;

#define COLOR_F 0xFFFF
#define COLOR_B 0x0000

void DrawSportTimer( int x, int y, int rtime)
{
	unsigned long temp;
	
	temp = (rtime/3600)%24;
	
	DrawUI_Bitmap_Ex( 0+8,140,8,16,Font_8x16[(temp/10)%10],COLOR_F);
	DrawUI_Bitmap_Ex( 8+8,140,8,16,Font_8x16[temp%10],COLOR_F);
	DrawUI_Bitmap_Ex(16+8,140,8,16,Font_8x16[11],COLOR_F);
	
	temp = (rtime/60)%60;
	
	DrawUI_Bitmap_Ex(24+8,140,8,16,Font_8x16[(temp/10)%10],COLOR_F);
	DrawUI_Bitmap_Ex(32+8,140,8,16,Font_8x16[temp%10],COLOR_F);	
	DrawUI_Bitmap_Ex(40+8,140,8,16,Font_8x16[11],COLOR_F);
	
    temp = rtime % 60;
    
    DrawUI_Bitmap_Ex(48+8,140,8,16,Font_8x16[(temp/10)%10],COLOR_F);
	DrawUI_Bitmap_Ex(56+8,140,8,16,Font_8x16[temp%10],COLOR_F);
}

void DrawCaloriesValue( int x, int y, int value )
{
    int count;
    
    if ( value > 99999 ){ value = 99999; }
    
    count = value;
    
    int width = 0;
    do
    {
      width += 8;
      count /= 10;
    }while( count > 0 );
    
    // x+((64-x) - width)/2; // Align center from x
    int sx = x; // Align left 
    
    int ex = sx + width - 8;
    do
    {
      DrawUI_Bitmap_Ex(ex, y, 8, 16, Font_8x16_Other[value%10],COLOR_F);	
      value /= 10;
      ex -= 8;  
    }while( value > 0 );  
}

void DrawSportIconBycle(void)
{	
	if ( sport_stat == 0 )
	{
		DrawUI_Bitmap_Ex(0,16,80,80,BMP_function_bycle,BMP_FUNCTION_BYCLE_RGB_F);		
		
		DrawUI_Bitmap_Ex(28,128,24,24,bmp_start, COLOR_F);
	}else{
		DrawUI_Bitmap_Ex(0,16,80,80,BMP_function_bycle,BMP_FUNCTION_BYCLE_RGB_F);			
		
		DrawUI_Bitmap_Ex(0, 112, 16, 16, bmp_caluire,COLOR_F);    // val
		DrawUI_Bitmap_Ex(56, 112, 24, 16, bmp_kc,COLOR_F);      // ICON

		DrawCaloriesValue(16,112, pedometer.calorie - config.train_bycle_calorie);
		
		DrawSportTimer(0,144,  getUTC() - config.train_bycle_utc);
	}
}

void DrawSportIconRunning(void)
{
	if ( sport_stat == 0 )
	{
		DrawUI_Bitmap_Ex(0,16,80,80,BMP_function_running,BMP_FUNCTION_RUNNING_RGB_F);	
		
		DrawUI_Bitmap_Ex(28,128,24,24,bmp_start, COLOR_F);
	}else{
		
		DrawUI_Bitmap_Ex(0,16,80,80,BMP_function_running,BMP_FUNCTION_RUNNING_RGB_F);	
		
		DrawUI_Bitmap_Ex(0, 112, 16, 16, bmp_caluire,COLOR_F);    // val
		DrawUI_Bitmap_Ex(56, 112, 24, 16, bmp_kc,COLOR_F);      // ICON

		DrawCaloriesValue(16,112,pedometer.calorie - config.train_running_calorie);		
		
		DrawSportTimer(0,144, getUTC() - config.train_running_utc );
	}		
}

void DrawSportIconSwiming(void)
{
	if ( sport_stat == 0 )
	{
		DrawUI_Bitmap_Ex((80-68)/2,16+16,68,48,bmp_function_swiming_b,BMP_FUNCTION_RUNNING_RGB_F);	
		
		DrawUI_Bitmap_Ex(28,128,24,24,bmp_start, COLOR_F);
	}else{
		
		DrawUI_Bitmap_Ex((80-54)/2,16+20,54,40,bmp_function_swiming_s,BMP_FUNCTION_RUNNING_RGB_F);	
		
		DrawUI_Bitmap_Ex(0, 112, 16, 16, bmp_caluire,COLOR_F);    // val
		DrawUI_Bitmap_Ex(56, 112, 24, 16, bmp_kc,COLOR_F);      // ICON

		DrawCaloriesValue(16,112,pedometer.calorie - config.train_sw_calorie);		
		
		DrawSportTimer(0,144, getUTC() - config.train_sw_utc );
	}    
}

void DrawSportIconBack(void)
{
	DrawUI_Bitmap_Ex(0,16,80,80,BMP_function_back, BMP_FUNCTION_BACK_RGB_F);	
	DrawUI_Bitmap_Ex(11,128,58,24,BMP_str_back, COLOR_F);	
}

void ui_win_sport(void)
{       
    switch ( sport_id )
    {
    case 0:				/* 自行车 */
        DrawSportIconBycle();
        break;
    case 1:				/* 跑步 */
        DrawSportIconRunning();
        break;
    case 2:             /* 游泳 */
        DrawSportIconSwiming();
        break;
    case 3:				/* 返回 */
        DrawSportIconBack();
        break;
    default:
        break;
    }
}

unsigned char select_enable;

unsigned long win_sport_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{
	static unsigned char leave_here = 0;
	
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0; 
		if ( leave_here == 0 )
		{
			sport_id = 0;
		}else{
			leave_here = 0;
		}
		
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
		
		memset( frame_buffer, 0, sizeof(frame_buffer));

		ui_win_sport();

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
			
			if ( sport_stat == 0 )
            {
                sport_id++; 
                sport_id %= SPORT_WIN_ID_NR;
            }
		}
		
		osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
		
		return ( msg_type ^ WINDOW_TOUCH );
	}
	
	if ( msg_type & WINDOW_PRESS )
	{	
        win_ticks = 0;
        
    	if ( config.enable == 0 )
    	{
            lcd_display(1);
            osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );

            return ( msg_type ^ WINDOW_PRESS );
    	}
        
    	if ( select_enable == 0 )
    	{
            select_enable = 1;
    	}else
        {
            if ( sport_id == 2 )
            {
              select_enable = 0;
            }
        }

    	if ( sport_stat == 1 && sport_id != 3 )
    	{
			if ( doubleTouchFlag != 1 )
			{
				return ( msg_type ^ WINDOW_PRESS );
			}             
            
            sport_stat = 0;

            // For update the lcd immediately.
            msg_type |= WINDOW_UPDATE;
          
            switch ( sport_id )
            {
            case 0:
                osal_set_event(taskStoreTaskId,TASK_STORE_SAVE_BYCLE_EVT);
                break;
            case 1:
                osal_set_event(taskStoreTaskId,TASK_STORE_SAVE_RUNNING_EVT);
                break;
            case 2:         
                osal_set_event(taskStoreTaskId,TASK_STORE_SAVE_SW_EVT);                
                break;
            }
            
            return ( msg_type ^ WINDOW_PRESS );
    	}

        switch ( sport_id )
        {
        case 0:				/* 自行车(开始) */
            sport_stat = 1;     
            config.train_bycle_utc = getUTC();
            config.train_bycle_pedometer = pedometer.counter;
            config.train_bycle_distance  = pedometer.distance;
            config.train_bycle_calorie   = pedometer.calorie;
            break;
        case 1:				/* 步行(开始) */
            sport_stat = 1;
            config.train_running_utc = getUTC();
            config.train_running_pedometer = pedometer.counter;
            config.train_running_distance  = pedometer.distance;
            config.train_running_calorie   = pedometer.calorie;
            break;
        case 2:				/* 步行(开始) */
            sport_stat = 1;
            config.train_sw_utc = getUTC();
            config.train_sw_pedometer = pedometer.counter;
            config.train_sw_distance  = pedometer.distance;
            config.train_sw_calorie   = pedometer.calorie;
            break;        
        case 3:				/* 返回 */
            sport_id = 0;
            sport_stat = 0;
            select_enable = 0;
			msg_type |= WINDOW_DISTORY;
            break;
        default:
            break;
        } 
        
        // For update the lcd immediately
        osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
		
		return ( msg_type ^ WINDOW_PRESS );
	}
	
	if ( msg_type & WINDOW_NOTIFY )
	{   
		leave_here = 1;
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
