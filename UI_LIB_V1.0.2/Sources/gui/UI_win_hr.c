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
#include "UI_sport_icon.h"
#include "OSAL_Timer.h"

#define COLOR_B      0X0000
#define COLOR_F      0xFFFF

#define HR_COLOR_B   0x0000
#define HR_COLOR_F

static unsigned char flag = 0;

void UI_Draw_Buttom(int index, int color);

void ui_win_hr_ready(void)
{
	if ( flag++ & 0x1)
	{
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_hr, 0x1F<<11); 
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_hr_1, 0xFFFF);	
	} else {
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_hr, 0x1F<<11); 
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_hr_2, 0xFFFF);	
	}
	
	for ( int y = 130-8; y < 133-8; y++ ) 
	for ( int x = 22; x < 37; x++ )
	{
		setpixel(x, y, 0xFFFF);
	}
	
	for ( int y = 130-8; y < 133-8; y++ ) 
	for ( int x = 42; x < 57; x++ )
	{
		setpixel(x, y, 0xFFFF);
	}	
}

void ui_win_hr_done(void)
{
	static unsigned char flag = 0;
	
	// 心率小图标
	if ( flag++ & 0x1 ){
		DrawUI_Bitmap( 10, 30, 16, 16, bmp_heart_rate, 0x1F<<11,0x0000);
	}	
	
	// 血压小图标
	DrawUI_Bitmap( 10, 80, 16, 16, bmp_bpressure, 0X072E,0x0000);
	// 血氧小图标
	DrawUI_Bitmap( 10, 130, 16, 16, bmp_spo2, 0x1F<<11,0x0000);
	
	// 心率值
	DrawUI_Bitmap(32, 24, 16,24,Font_16x24[(hr.hr/100)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(48, 24, 16,24,Font_16x24[(hr.hr/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(64, 24, 16,24,Font_16x24[hr.hr%10],COLOR_F,COLOR_B);
	
	// 血压值
	#if 0
	DrawUI_Bitmap( 72, 80, 8, 16, Font_8x16_Other[hr.hp%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 64, 80, 8, 16, Font_8x16_Other[(hr.hp/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 56, 80, 8, 16, Font_8x16_Other[hr.hp/100],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 48, 80, 8, 16, Font_8x16_Other[11],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 40, 80, 8, 16, Font_8x16_Other[hr.lp%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 32, 80, 8, 16, Font_8x16_Other[hr.lp/10],COLOR_F,COLOR_B);	
    #else
	DrawUI_Bitmap( 48, 80, 8, 16, Font_8x16_Other[hr.hp%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 40, 80, 8, 16, Font_8x16_Other[(hr.hp/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 32, 80, 8, 16, Font_8x16_Other[hr.hp/100],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 56, 80, 8, 16, Font_8x16_Other[11],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 72, 80, 8, 16, Font_8x16_Other[hr.lp%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 64, 80, 8, 16, Font_8x16_Other[hr.lp/10],COLOR_F,COLOR_B);
	#endif	
	
	// 血压单位
	DrawUI_Bitmap( 80-32, 80+16, 32, 10, gImage_mmhg_hr,COLOR_F,COLOR_B);
	
	// 血氧值
	DrawUI_Bitmap( 48, 130, 8, 16, Font_8x16_Other[((int)hr.spo2)/10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 56, 130, 8, 16, Font_8x16_Other[((int)(hr.spo2))%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 64, 130, 16, 16, bmp_unite_pecent,COLOR_F,COLOR_B);
}

void ui_win_hr_stop(void)
{
	// 心率小图标
	DrawUI_Bitmap( 10, 30, 16, 16, bmp_heart_rate, 0x1F<<11,0x0000);
	
	// 血压小图标
	DrawUI_Bitmap( 10, 80, 16, 16, bmp_bpressure, 0X072E,0x0000);
	// 血氧小图标
	DrawUI_Bitmap( 10, 130, 16, 16, bmp_spo2, 0x1F<<11,0x0000);
	
	// 心率值
	DrawUI_Bitmap(32, 24, 16,24,Font_16x24[(hr.hr/100)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(48, 24, 16,24,Font_16x24[(hr.hr/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap(64, 24, 16,24,Font_16x24[hr.hr%10],COLOR_F,COLOR_B);
	
	// 血压值
	#if 0
	DrawUI_Bitmap( 72, 80, 8, 16, Font_8x16_Other[hr.hp%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 64, 80, 8, 16, Font_8x16_Other[(hr.hp/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 56, 80, 8, 16, Font_8x16_Other[hr.hp/100],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 48, 80, 8, 16, Font_8x16_Other[11],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 40, 80, 8, 16, Font_8x16_Other[hr.lp%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 32, 80, 8, 16, Font_8x16_Other[hr.lp/10],COLOR_F,COLOR_B);	
    #else
	DrawUI_Bitmap( 48, 80, 8, 16, Font_8x16_Other[hr.hp%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 40, 80, 8, 16, Font_8x16_Other[(hr.hp/10)%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 32, 80, 8, 16, Font_8x16_Other[hr.hp/100],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 56, 80, 8, 16, Font_8x16_Other[11],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 72, 80, 8, 16, Font_8x16_Other[hr.lp%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 64, 80, 8, 16, Font_8x16_Other[hr.lp/10],COLOR_F,COLOR_B);
	#endif
	
	// 血压单位
	DrawUI_Bitmap( 80-32, 80+16, 32, 10, gImage_mmhg_hr,COLOR_F,COLOR_B);
	
	// 血氧值
	DrawUI_Bitmap( 48, 130, 8, 16, Font_8x16_Other[((int)hr.spo2)/10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 56, 130, 8, 16, Font_8x16_Other[((int)(hr.spo2))%10],COLOR_F,COLOR_B);
	DrawUI_Bitmap( 64, 130, 16, 16, bmp_unite_pecent,COLOR_F,COLOR_B);
}

const unsigned char hr_error[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x30,0x00,0x0E,0x70,0x00,0x07,0xE0,0x80,0x03,0xC0,0xC1,0x01,0x80,0xE3,0x00,0x00,0x77,0x00,0x00,0x3E,0x00,0x00,0x1C,0x00,0x00,0x3E,0x00,0x00,0x7F,0x00,0x80,0xE3,0x00,0xC0,0xC1,0x01,0xE0,0x80,0x03,0x70,0x00,0x07,0x38,0x00,0x0E,0x10,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};/*"F:\BandAll\HB018\HB018切图\小图标\failure.bmp",0*/

void ui_win_hr_error(void)
{
	DrawUI_Bitmap_Ex(4,16,72,72,BMP_hr, 0x1F<<11); 
	DrawUI_Bitmap_Ex(4,16,72,72,BMP_hr_1, 0xFFFF);
	
	DrawUI_Bitmap( (80-24)/2, 130, 24, 24, hr_error,COLOR_F,COLOR_B);
}

void ui_win_hr_none(void)
{
	DrawUI_Bitmap_Ex(4,16,72,72,BMP_hr, 0x1F<<11); 
	DrawUI_Bitmap_Ex(4,16,72,72,BMP_hr_1, 0xFFFF);	
	
	#if 0
	// 心率值
	if ( hr.hr > 99 )
	{
		x = (80 - 16*3)/2;
		DrawUI_Bitmap( x, 130, 16,24,Font_16x24[(hr.hr/100)%10],COLOR_F,COLOR_B);
		x += 16;
	}else{
		x = (80 - 16*2)/2;
	}
	
	DrawUI_Bitmap( x, 130, 16,24,Font_16x24[(hr.hr/10)%10],COLOR_F,COLOR_B);
	x += 16;
	DrawUI_Bitmap( x, 130, 16,24,Font_16x24[hr.hr%10],COLOR_F,COLOR_B);
	#else
	for ( int y = 130-8; y < 133-8; y++ ) 
	for ( int x = 22; x < 37; x++ )
	{
		setpixel(x, y, 0xFFFF);
	}
	
	for ( int y = 130-8; y < 133-8; y++ ) 
	for ( int x = 42; x < 57; x++ )
	{
		setpixel(x, y, 0xFFFF);
	}	
	#endif
}

unsigned long win_hr_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{	
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0; last_hr = 0; need_mTicks = 8000;
		to_close = 0;
        
		draw_fill(0,0,80,160,COLOR_B);

		hr.stat = HR_DECT_STAT_NONE;		
		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
		}
		
		osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
		
		// Start measure the hr after 800ms if stay on the window still
		
		osal_start_timerEx( taskGUITaskId, TASK_GUI_HR_DELAY_EVT, 1000); 
		
		return ( msg_type ^ WINDOW_CREATE );
	}
	
	if ( msg_type & WINDOW_UPDATE )
	{
		win_ticks++; to_close++;

        if ( (win_ticks > 250) || ((hr.stop == 1) && (to_close > 5)) )
        {
            lcd_display(0);
            
            return ( msg_type ^ WINDOW_UPDATE );
        }
		
		draw_fill(0,0,80,160,COLOR_B);
	
        switch ( hr.stat )
        {         
        case HR_DECT_STAT_NONE:
            ui_win_hr_none();
			UI_Draw_Buttom(3,0x1F<<11);
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
		
		osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 650);
		
		if ( last_hr != hr.hr || hr.stat == HR_DECT_STAT_NORMAL
				|| hr.stat == HR_DECT_STAT_DONE
				|| hr.stat == HR_DECT_STAT_ERROR 
				|| hr.stat == HR_DECT_STAT_NONE
				|| hr.stat == HR_DECT_STAT_STOP
				|| hr_showFlag == 1 )
		{		
			sync_frame();
			
			last_hr = hr.hr; hr_showFlag = 0;
		}
		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(true);
		}
				
		return ( msg_type ^ WINDOW_UPDATE );
	}
	
	if ( msg_type & WINDOW_TOUCH )
	{	
		#if 0 /** 客户定制的功能,进入直接测,单击停止并切换 */
		if ( hr.stat != HR_DECT_STAT_ERROR
			&& hr.stat != HR_DECT_STAT_NONE 
			&& hr.stat != HR_DECT_STAT_STOP
			&& hr.stop != 1)
		{
			return ( msg_type ^ WINDOW_TOUCH );
		}		
		#endif
		
		if ( config.lcdEnable == 0)
		{
			win_ticks = 0; to_close = 0;
			msg_type |= WINDOW_UPDATE;
		}else{
			window_push(window_get(win_current));
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
