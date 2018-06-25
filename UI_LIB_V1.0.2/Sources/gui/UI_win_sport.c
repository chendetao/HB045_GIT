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

#define COLOR_B      0X0000
#define COLOR_F      0xFFFF

#define SPORT_WIN_ID_NR      5

unsigned long sport_id, sport_stat;
unsigned char flag;

void UI_Draw_Buttom(int index, int color);

#include "UI_comm_icon.h"
#define SWM_YES  1
#define SWM_NO   0
unsigned char swm_in_exit_mode = 0;
unsigned char swm_yes_no = SWM_YES;

const unsigned char exit_str[][16] = 
{
{0x08,0x20,0xF8,0x3F,0x88,0x20,0x88,0x20,0xE8,0x23,0x08,0x20,0x10,0x18,0x00,0x00},/*"E",0*/

{0x00,0x00,0x80,0x20,0x80,0x31,0x00,0x2E,0x80,0x0E,0x80,0x31,0x80,0x20,0x00,0x00},/*"x",1*/

{0x00,0x00,0x80,0x20,0x98,0x20,0x98,0x3F,0x00,0x20,0x00,0x20,0x00,0x00,0x00,0x00},/*"i",2*/

{0x00,0x00,0x80,0x00,0x80,0x00,0xE0,0x1F,0x80,0x20,0x80,0x20,0x00,0x00,0x00,0x00},/*"t",3*/

{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*" ",4*/

{0x00,0x00,0x70,0x00,0x48,0x00,0x08,0x30,0x08,0x36,0x08,0x01,0xF0,0x00,0x00,0x00},/*"?",5*/    
};

void show_swm_exit_comfirm(void)
{
    for ( int i = 0; i < 6; i++ )
    {
        DrawUI_Bitmap_Ex( i*8+16, 116, 8, 16, exit_str[i],0xFFFF);
    }
    
    if ( swm_yes_no == SWM_YES )
    {
        DrawUI_Bitmap_Ex(9,144,24,16,yes_no_bg,0x1F<<11);
    }else{
        DrawUI_Bitmap_Ex(50,144,24,16,yes_no_bg,0x1F<<11);
    }
    
    for ( int i = 0; i < 3; i++ )
    {
        DrawUI_Bitmap_Ex( i*7+10, 144, 7, 16, yes[i],0xFFFF);
    }
    
    for ( int i = 0; i < 2; i++ )
    {
        DrawUI_Bitmap_Ex( i*7+56, 144, 7, 16, no[i],0xFFFF);
    }     
}

void DrawSportTimer( int x, int y, int rtime)
{
	unsigned long temp;
	
	temp = (rtime/3600)%100;
	
	DrawUI_Bitmap( 0+8,140,8,16,Font_8x16[(temp/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 8+8,140,8,16,Font_8x16[temp%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(16+8,140,8,16,Font_8x16[11],COLOR_F,COLOR_B);
	
	temp = (rtime/60)%60;
	
	DrawUI_Bitmap(24+8,140,8,16,Font_8x16[(temp/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(32+8,140,8,16,Font_8x16[temp%10],COLOR_F,COLOR_B);	
	DrawUI_Bitmap(40+8,140,8,16,Font_8x16[11],COLOR_F,COLOR_B);
	
    temp = rtime % 60;
    
    DrawUI_Bitmap(48+8,140,8,16,Font_8x16[(temp/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(56+8,140,8,16,Font_8x16[temp%10],COLOR_F,COLOR_B);
}

void DrawCaloriesValue( int x, int y, int value )
{
    int count;
    
    if ( value > 99999 ){ value = 99999; }
    
    count = value;
    
    int width = 0;
    do
    {
      width += 12;
      count /= 10;
    }while( count > 0 );
    
    // x+((64-x) - width)/2; // Align center from x
    int sx = x; // Align left 
    
    int ex = sx + width - 12;
    do                                    
    {
      DrawUI_Bitmap_Ex(ex, y, 11, 24, Font_11x24[value%10],0xFFFF);	
      value /= 10;
      ex -= 12;  
    }while( value > 0 );  
}

#define BYCLE_COLOR_B 0x0000
#define BYCLE_COLOR_F 0X072E 

void DrawSportIconMain(void)
{	
	draw_fill(0,0,80,160,COLOR_B);
	
	static int c = 0;
	
	switch ( c )	
	{
	case 0:
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_bycle_1, 0XFEA0);
		c = 1;
		break;
	case 1:
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_bycle_2, 0XFEA0);
		c = 2;
		break;
	case 2:
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_bycle_3, 0XFEA0);
		c = 0;
		break;
	default:
		break;
	}
	
	DrawUI_Bitmap_Ex(16,116,48,16,BMP_str_train, 0xFFFF);
}

void DrawSportIconBycle(void)
{
	draw_fill(0,0,80,160,COLOR_B);
	
	if ( sport_stat == 0 )
	{
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_bycle_4, 0XFC80);
		
		DrawUI_Bitmap(28,128,24,24,bmp_start, COLOR_F,COLOR_B);
	}else{		
		static int c = 0;
		
		switch ( c )	
		{
		case 0:
			DrawUI_Bitmap_Ex(4,16,72,72,BMP_bycle_4, 0XFC80);
			c = 1;
			break;
		case 1:
			DrawUI_Bitmap_Ex(4,16,72,72,BMP_bycle_5, 0XFC80);
			c = 2;
			break;
		case 2:
			DrawUI_Bitmap_Ex(4,16,72,72,BMP_bycle_6, 0XFC80);
			c = 0;
			break;
		default:
			break;
		}
		
		DrawUI_Bitmap(56, 112, 24, 16, bmp_kc,COLOR_F,COLOR_B);      // unit

		DrawCaloriesValue(8,96,pedometer.calorie - config.train_bycle_calorie);
		
		DrawSportTimer(0,144,  getUTC() - config.train_bycle_utc);
	}
}

void DrawSportIconRunning(void)
{
	draw_fill(0,0,LCD_W,LCD_H,COLOR_B);
	
	if ( sport_stat == 0 )
	{
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_running_1, 0XFEA0);
		
		DrawUI_Bitmap_Ex(28,128,24,24,bmp_start, 0xFFFF);
	}else{
		
		static int c = 0;
		
		switch ( c )	
		{
		case 0:
			DrawUI_Bitmap_Ex(4,16,72,72,BMP_running_1, 0XFEA0);
			c = 1;
			break;
		case 1:
			DrawUI_Bitmap_Ex(4,16,72,72,BMP_running_2, 0XFEA0);
			c = 2;
			break;
		case 2:
			DrawUI_Bitmap_Ex(4,16,72,72,BMP_running_3, 0XFEA0);
			c = 0;
			break;
		default:
			break;
		}
		

        DrawUI_Bitmap(56, 120, 24, 16, bmp_kc,COLOR_F,COLOR_B);      // unit

        DrawCaloriesValue( 8,96,pedometer.calorie - config.train_running_calorie);		
        
        DrawSportTimer(0,144, getUTC() - config.train_running_utc );
	}		
}

void DrawSportIconSwimming(void)
{
	draw_fill(0,0,LCD_W,LCD_H,COLOR_B);
	
	if ( sport_stat == 0 )
	{
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_swiming_1, 0X073F);
		
		DrawUI_Bitmap(28,128,24,24,bmp_start, 0xFFFF,0x0000);
	}else{
		
		if ( flag++ & 0x1 )
		{
			DrawUI_Bitmap_Ex(4,16,72,72,BMP_swiming_1, 0X073F);
		}else{
			DrawUI_Bitmap_Ex(4,16,72,72,BMP_swiming_2, 0X073F);
		}
		
        if ( swm_in_exit_mode == 0 )
        {
            DrawUI_Bitmap(56, 120, 24, 16, bmp_kc,COLOR_F,COLOR_B);      // unit
            DrawCaloriesValue(8,96,(int)((pedometer.calorie - config.train_swimming_calorie)*2.15f));				
            DrawSportTimer(0,144, getUTC() - config.train_swimming_utc );
        }else{
            show_swm_exit_comfirm();
        }
	}		
}

#define BACK_RGB_F 0X2BDF
#define BACK_RGB_B 0X0000

void DrawSportIconBack(void)
{
	draw_fill(0,0,LCD_W,LCD_H,COLOR_B);
	
	DrawUI_Bitmap_Ex(4,16,72,72,BMP_back, 0xFFFF);	
	DrawUI_Bitmap(0,120,80,16,bmp_str_back, COLOR_F,BACK_RGB_B);	
}

void ui_win_sport(void)
{       
    switch ( sport_id )
    {
    case 0:             /* 运动界面(主界面) */
    	DrawSportIconMain();
		UI_Draw_Buttom(2,0XFEA0);
        break;
    case 1:				/* 自行车 */
        DrawSportIconBycle();
        break;
    case 2:				/* 跑步 */
        DrawSportIconRunning();
        break;
	case 3:             /* 游泳 */
		DrawSportIconSwimming();
		break;
    case 4:				/* 返回 */
        DrawSportIconBack();
        break;
    default:
        break;
    }
}

unsigned char select_enable = 0;
unsigned char notify_flag = 0;

unsigned long win_sport_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{	
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0;  
		
		if ( notify_flag == 0 )
		{
			sport_id = 0;  swm_in_exit_mode = 0; swm_yes_no = SWM_NO;
		} else {
			notify_flag = 0;
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
			
            if ( select_enable == 0 )
            {
                window_push(window_get(win_current));
                msg_type |= WINDOW_DISTORY;
            }else if ((sport_id == 3) && (sport_stat == 1))
            {
                if ( swm_in_exit_mode == 1 )
                {
                    swm_yes_no = (swm_yes_no==SWM_YES)?SWM_NO:SWM_YES;
                }
            }
            else if ( sport_stat == 0 )
            {
                sport_id++; 
                sport_id %= SPORT_WIN_ID_NR;
                if ( sport_id == 0 )
                {
                  sport_id = 1;
                }
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
            if ( sport_id == 4 )
            {
              select_enable = 0;
            }
        }

    	if ( sport_stat == 1 && sport_id != 4 )
    	{
            if ( sport_id == 3 )
            {
                if ( swm_in_exit_mode == 0 )
                {
                    swm_in_exit_mode = 1;
                    return ( msg_type ^ WINDOW_PRESS );
                }else{
                    if ( swm_yes_no == SWM_NO )
                    {
                        swm_in_exit_mode = 0;
                        return ( msg_type ^ WINDOW_PRESS );
                    }
                }
            }
            swm_in_exit_mode = 0; swm_yes_no = SWM_NO;
            
            
            sport_stat = 0;

            // For update the lcd immediately.
            msg_type |= WINDOW_UPDATE;
          
            switch ( sport_id )
            {
            case 1:
                osal_set_event(taskStoreTaskId,TASK_STORE_SAVE_BYCLE_EVT);
                break;
            case 2:
                osal_set_event(taskStoreTaskId,TASK_STORE_SAVE_RUNNING_EVT);
				break;
			case 3:
				osal_set_event(taskStoreTaskId,TASK_STORE_SAVE_SWIMMING_EVT); 
                break;
            }
            
            return ( msg_type ^ WINDOW_PRESS );
    	}

        switch ( sport_id )
        {
        case 0:             /* 运动界面(主界面) */	
            sport_id = 1;
            break;
        case 1:				/* 自行车(开始) */
            sport_stat = 1;     
            config.train_bycle_utc = getUTC();
            config.train_bycle_pedometer = pedometer.counter;
            config.train_bycle_distance  = pedometer.distance;
            config.train_bycle_calorie   = pedometer.calorie;
            break;
        case 2:				/* 步行(开始) */
            sport_stat = 1;
            config.train_running_utc = getUTC();
            config.train_running_pedometer = pedometer.counter;
            config.train_running_distance  = pedometer.distance;
            config.train_running_calorie   = pedometer.calorie;
            break;
		case 3:				/* 游泳(开始)*/
            sport_stat = 1;
            config.train_swimming_utc = getUTC();
            config.train_swimming_pedometer = pedometer.counter;
            config.train_swimming_distance  = pedometer.distance;
            config.train_swimming_calorie   = pedometer.calorie;	
			break;
        case 4:				/* 返回 */
            sport_id = 0;
            sport_stat = 0;
            select_enable = 0;
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
		notify_flag = 1;
		
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
