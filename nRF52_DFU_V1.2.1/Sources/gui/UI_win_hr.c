#include "string.h"
#include "Window.h"

#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_GUI.h"
#include "Task_Hr.h"

#include "LCD_ST7735.h"
#include "QR_Utils.h"

#include "UI_win_hr.h"
#include "UserSetting.h"
#include "UI_Draw.h"

#include "UI_hr_icon.h"
#include "UI_comm_icon.h"
#include "UI_Comm_Draw.h"
#include "UI_main_icon.h"
#include "UI_function_icon.h"


#define COLOR_F  0xFFFF
#define COLOR_B  0x0000

void Draw_Hr_Value( int x, int y, int val, int flag)
{
	int width = 0;
	
	int v = val;
	do{
		if ( (v % 10) == 1 ){
			width += 4;
		} else {
			width += 11;
		}
		v /= 10;
	}while( v > 0 );
	
	if ( val < 100 )
	{
		width += 11;
	}
	
	if ( flag == 0 )
	{
		x = (LCD_W - width)/2;
	}else{
		x = LCD_W-width;
	}
	
	int idx = (val/100) % 10;
	if ( idx == 1 )
	{
		DrawUI_Bitmap_Ex(x,y,4,24,Font_11x24[idx], COLOR_F);
		x += 4;
	} else {
		DrawUI_Bitmap_Ex(x,y,11,24,Font_11x24[idx], COLOR_F);
		x += 11;
	}
	
	idx = (val/10) % 10;
	if ( idx == 1 )
	{
		DrawUI_Bitmap_Ex(x,y,4,24,Font_11x24[idx], COLOR_F);
        x += 4;
	} else {
		DrawUI_Bitmap_Ex(x,y,11,24,Font_11x24[idx], COLOR_F);
		x += 11;
	}

	idx = val % 10;
	if ( idx == 1 )
	{
		DrawUI_Bitmap_Ex(x,y,4,24,Font_11x24[idx], COLOR_F);
		x += 4;
	} else {
		DrawUI_Bitmap_Ex(x,y,11,24,Font_11x24[idx], COLOR_F);
		x += 11;
	}	
}

void ui_win_hr_ready(void)
{
	DrawUI_Bitmap(0,16,80,80,BMP_hr_1, 0x1F<<11,0x00); 
	if ( config.showFlag++ & 0x1)
	{
		DrawUI_Bitmap_Ex(0,16,80,80,BMP_hr_2, 0xFFFF);
	} else {
		DrawUI_Bitmap_Ex(0,16,80,80,BMP_hr_3, 0xFFFF);
	}
	
	for ( int y = 130; y < 133; y++ ) 
	for ( int x = 22; x < 37; x++ )
	{
		set_pixel(x, y, 0xFFFF);
	}
	
	for ( int y = 130; y < 133; y++ ) 
	for ( int x = 42; x < 57; x++ )
	{
		set_pixel(x, y, 0xFFFF);
	}	
}

void ui_win_hr_done(void)
{
	static unsigned char flag = 0;
	
	if ( flag++ & 0x1 ){
		draw_bitmap( 10, 30, 16, 16, gImage_hr_done);
	}	
	draw_bitmap( 10, 80, 16, 16, gImage_bp_done);
	draw_bitmap( 10, 130, 16, 16, gImage_o2_done);
	
	Draw_Hr_Value(0,28,hr.hr,1);
	
	#if 0
	DrawUI_Bitmap(72,80, 7, 16,Font_7x16[hr.hp%10], COLOR_F,COLOR_B);
	DrawUI_Bitmap(65,80, 7, 16,Font_7x16[(hr.hp/10)%10], COLOR_F,COLOR_B);
	DrawUI_Bitmap(58,80, 7, 16,Font_7x16[hr.hp/100], COLOR_F,COLOR_B);
	DrawUI_Bitmap(51,80, 7, 16,Font_7x16[12], COLOR_F,COLOR_B);
	DrawUI_Bitmap(44,80, 7, 16,Font_7x16[hr.lp%10], COLOR_F,COLOR_B);
	DrawUI_Bitmap(37,80, 7, 16,Font_7x16[hr.lp/10], COLOR_F,COLOR_B);
	#else
	DrawUI_Bitmap_Ex(51,80, 7, 16,Font_7x16[hr.hp%10], COLOR_F);
	DrawUI_Bitmap_Ex(44,80, 7, 16,Font_7x16[(hr.hp/10)%10], COLOR_F);
	DrawUI_Bitmap_Ex(37,80, 7, 16,Font_7x16[hr.hp/100], COLOR_F);
	DrawUI_Bitmap_Ex(58,80, 7, 16,Font_7x16[12], COLOR_F);
	DrawUI_Bitmap_Ex(72,80, 7, 16,Font_7x16[hr.lp%10], COLOR_F);
	DrawUI_Bitmap_Ex(65,80, 7, 16,Font_7x16[hr.lp/10], COLOR_F);
	#endif
	
	draw_bitmap( 80-32, 80+16, 32, 10, gImage_mmhg_hr);

	DrawUI_Bitmap_Ex( 44, 130, 7, 16, Font_7x16[hr.sp_h/10], COLOR_F);	
	DrawUI_Bitmap_Ex( 51, 130, 7, 16, Font_7x16[hr.sp_h%10], COLOR_F);
	DrawUI_Bitmap_Ex( 58, 130, 7, 16, Font_7x16[10], COLOR_F);	
	DrawUI_Bitmap_Ex( 65, 130, 7, 16, Font_7x16[hr.sp_l], COLOR_F);	
	DrawUI_Bitmap_Ex( 72, 130, 7, 16, Font_7x16[13], COLOR_F);
}


void ui_win_hr_stop(void)
{
	draw_bitmap( 10, 30, 16, 16, gImage_hr_done);
	draw_bitmap( 10, 80, 16, 16, gImage_bp_done);
	draw_bitmap( 10, 130, 16, 16, gImage_o2_done);
	
	Draw_Hr_Value(0,28,hr.hr,1);
	
	#if 0
	DrawUI_Bitmap(72,80, 7, 16,Font_7x16[hr.hp%10], COLOR_F,COLOR_B);
	DrawUI_Bitmap(65,80, 7, 16,Font_7x16[(hr.hp/10)%10], COLOR_F,COLOR_B);
	DrawUI_Bitmap(58,80, 7, 16,Font_7x16[hr.hp/100], COLOR_F,COLOR_B);
	DrawUI_Bitmap(51,80, 7, 16,Font_7x16[12], COLOR_F,COLOR_B);
	DrawUI_Bitmap(44,80, 7, 16,Font_7x16[hr.lp%10], COLOR_F,COLOR_B);
	DrawUI_Bitmap(37,80, 7, 16,Font_7x16[hr.lp/10], COLOR_F,COLOR_B);
	#else
	DrawUI_Bitmap_Ex(51,80, 7, 16,Font_7x16[hr.hp%10], COLOR_F);
	DrawUI_Bitmap_Ex(44,80, 7, 16,Font_7x16[(hr.hp/10)%10], COLOR_F);
	DrawUI_Bitmap_Ex(37,80, 7, 16,Font_7x16[hr.hp/100], COLOR_F);
	DrawUI_Bitmap_Ex(58,80, 7, 16,Font_7x16[12], COLOR_F);
	DrawUI_Bitmap_Ex(72,80, 7, 16,Font_7x16[hr.lp%10], COLOR_F);
	DrawUI_Bitmap_Ex(65,80, 7, 16,Font_7x16[hr.lp/10], COLOR_F);
	#endif
	
	draw_bitmap( 80-32, 80+16, 32, 10, gImage_mmhg_hr);

	DrawUI_Bitmap_Ex( 44, 130, 7, 16, Font_7x16[hr.sp_h/10], COLOR_F);	
	DrawUI_Bitmap_Ex( 51, 130, 7, 16, Font_7x16[hr.sp_h%10], COLOR_F);
	DrawUI_Bitmap_Ex( 58, 130, 7, 16, Font_7x16[10], COLOR_F);	
	DrawUI_Bitmap_Ex( 65, 130, 7, 16, Font_7x16[hr.sp_l], COLOR_F);	
	DrawUI_Bitmap_Ex( 72, 130, 7, 16, Font_7x16[13], COLOR_F);
}

void ui_win_hr_error(void)
{
	DrawUI_Bitmap(0,16,80,80,BMP_hr_1, 0x1F<<11,0x00); 
	DrawUI_Bitmap_Ex(0,16,80,80,BMP_hr_2, 0xFFFF);		
	
	DrawUI_Bitmap((80-24)/2,130,24,24,bmp_error, 0x1F<<11,0x00); 	
}

void ui_win_hr_none(void)
{
	DrawUI_Bitmap(0,16,80,80,BMP_hr_1, 0x1F<<11,0x00); 
	DrawUI_Bitmap_Ex(0,16,80,80,BMP_hr_2, 0xFFFF);
	
	Draw_Hr_Value(42,128,hr.hr, 0);
}

unsigned char leave_here = 0;

unsigned long win_hr_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{	
	if ( msg_type & WINDOW_CREATE )
	{
        if ( leave_here == 0 )
        {
            need_mTicks = 8000;
            to_close = 0;
                    
            // Add for heart measure when entry this window
            // (2017.09.09)
            
            {
                hr.stat = HR_DECT_STAT_NORMAL;
                //  Delay 100ms for window update Immediately
                osal_start_timerEx ( taskHrTaskId, TASK_HR_START_EVT, 100);
            }  
        }else{
            leave_here = 0;
        }
        
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
		win_ticks++; to_close++;

        if ( (win_ticks > 250) || ((hr.stop == 1) && (to_close > 6)) )
        {
            lcd_display(0);
            
            return ( msg_type ^ WINDOW_UPDATE );
        }
		
		memset(frame_buffer, 0, sizeof(frame_buffer));
	
        switch ( hr.stat )
        {         
        case HR_DECT_STAT_NONE:
            ui_win_hr_none();
            break;			
        case HR_DECT_STAT_STOP:
			ui_win_hr_stop();
			break;
        case HR_DECT_STAT_NORMAL:
            ui_win_hr_ready();
            break;
        case HR_DECT_STAT_ERROR:
            ui_win_hr_error();
            break;
        case HR_DECT_STAT_DONE:
            ui_win_hr_done();
            break;
        default:
            break;
        }
		
		osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 500);
		
		if ( hr.stat == HR_DECT_STAT_NORMAL
				|| hr.stat == HR_DECT_STAT_DONE
				|| hr.stat == HR_DECT_STAT_ERROR 
				|| hr.stat == HR_DECT_STAT_NONE
				|| hr.stat == HR_DECT_STAT_STOP )
		{		
			sync_frame();
		}
				      
		return ( msg_type ^ WINDOW_UPDATE );
	}
	
	if ( msg_type & WINDOW_TOUCH )
	{	
		if ( hr.stat != HR_DECT_STAT_ERROR
			&& hr.stat != HR_DECT_STAT_NONE 
			&& hr.stat != HR_DECT_STAT_STOP
			&& hr.stop != 1)
		{
			return ( msg_type ^ WINDOW_TOUCH );
		}		
		
		if ( config.lcdEnable == 0)
		{
			lcd_display(1);
			win_ticks = 0; to_close = 0;
			msg_type |= WINDOW_UPDATE;
		}else{
			msg_type |= WINDOW_DISTORY;
		}			
		
		return ( msg_type ^ WINDOW_TOUCH );
	}
	
	if ( msg_type & WINDOW_PRESS )
	{
		win_ticks = 0; to_close = 0;
		
		if ( config.lcdEnable == 0)
		{
			lcd_display(1);

			msg_type |= WINDOW_UPDATE;
			
			return ( msg_type ^ WINDOW_PRESS );
		}
		
		if ( hr.stop == 1 )
		{
			need_mTicks = 8000;
			osal_set_event ( taskHrTaskId, TASK_HR_START_EVT );
		} else
		{
			hr.stop = 1;
			if ( hr.stat == HR_DECT_STAT_DONE )
			{
				hr.stat = HR_DECT_STAT_STOP;
			}else{
				hr.stat = HR_DECT_STAT_NONE;
			}
			osal_set_event ( taskHrTaskId, TASK_HR_STOP_EVT );
		}
		
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
		if ( hr.stop != 1 )
		{
			osal_set_event ( taskHrTaskId, TASK_HR_STOP_EVT );
		}
		
		if ( hr.stat != HR_DECT_STAT_NONE )
		{
			hr.stat = HR_DECT_STAT_NONE;
		}
		
		window_pop(&win_current);
		
		return window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0 );
	}

    return 0;
}
