
#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_GUI.h"
#include "Task_Step.h"
#include "LCD_ST7735.h"

#include "string.h"
#include "Window.h"
#include "UserSetting.h"
#include "UI_Draw.h"

#include "UI_info_icon.h"
#include "UI_comm_icon.h"
#include "UI_Comm_Draw.h"
#include "UI_main_icon.h"
#include "UI_win_info.h"
#include "UI_alarm_icon.h"
#include "HFMA2Ylib.h"
#include "version.h"

#define COLOR_B      0X0000
#define COLOR_F      0X072E

extern unsigned char mac_buf[18];
extern unsigned char DZ[32];
extern void DoRect8x16( int cx, int cy, const unsigned char pic[32]);
unsigned char showMacFlag;

void UI_Draw_Buttom(int index, int color);

extern void EntryDeepSleepMode(void);
extern void ExitDeepSleepMode(void);

void show_information_val(int x, int y )
{	
	// "MAC"
	ASCII_GetData('M',ASCII_8X16, DZ);
	DoRect8x16(28, 24, DZ);
	ASCII_GetData('A',ASCII_8X16, DZ);
	DoRect8x16(36, 24, DZ);
	ASCII_GetData('C',ASCII_8X16, DZ);
	DoRect8x16(44, 24, DZ);	
	
	// address
	ASCII_GetData(mac_buf[0],ASCII_8X16, DZ);
	DoRect8x16( 8, 64, DZ);
	ASCII_GetData(mac_buf[1],ASCII_8X16, DZ);
	DoRect8x16(16, 64, DZ);
	ASCII_GetData(mac_buf[2],ASCII_8X16, DZ);
	DoRect8x16(24, 64, DZ);
	ASCII_GetData(mac_buf[3],ASCII_8X16, DZ);
	DoRect8x16(32, 64, DZ);
	ASCII_GetData(mac_buf[4],ASCII_8X16, DZ);
	DoRect8x16(40, 64, DZ);
	ASCII_GetData(mac_buf[5],ASCII_8X16, DZ);
	DoRect8x16(48, 64, DZ);
	ASCII_GetData(mac_buf[6],ASCII_8X16, DZ);
	DoRect8x16(56, 64, DZ);
	ASCII_GetData(mac_buf[7],ASCII_8X16, DZ);
	DoRect8x16(64, 64, DZ);
	
	ASCII_GetData(mac_buf[9],ASCII_8X16, DZ);
	DoRect8x16( 8, 80, DZ);
	
	ASCII_GetData(mac_buf[10],ASCII_8X16, DZ);
	DoRect8x16(16, 80, DZ);
	ASCII_GetData(mac_buf[11],ASCII_8X16, DZ);
	DoRect8x16(24, 80, DZ);
	ASCII_GetData(mac_buf[12],ASCII_8X16, DZ);
	DoRect8x16(32, 80, DZ);
	ASCII_GetData(mac_buf[13],ASCII_8X16, DZ);
	DoRect8x16(40, 80, DZ);
	ASCII_GetData(mac_buf[14],ASCII_8X16, DZ);
	DoRect8x16(48, 80, DZ);
	ASCII_GetData(mac_buf[15],ASCII_8X16, DZ);
	DoRect8x16(56, 80, DZ);
	ASCII_GetData(mac_buf[16],ASCII_8X16, DZ);
	DoRect8x16(64, 80, DZ);		
}

void ui_show_version(void)
{  
    ASCII_GetData('V',ASCII_8X16,DZ); //Vx.x.x
    DoRect8x16(16, 120, DZ);              
    ASCII_GetData('0'+VERSION_MAJOR,ASCII_8X16,DZ);
    DoRect8x16(24, 120, DZ);
    ASCII_GetData('.',ASCII_8X16,DZ);
    DoRect8x16(32, 120, DZ);    
    ASCII_GetData('0'+VERSION_MINOR,ASCII_8X16,DZ);
    DoRect8x16(40, 120, DZ);
    ASCII_GetData('.',ASCII_8X16,DZ);
    DoRect8x16(48, 120, DZ);    
    ASCII_GetData('0'+VERSION_REVISON,ASCII_8X16,DZ);
    DoRect8x16(56, 120, DZ);      
}

void show_information(int x, int y )
{	
	draw_fill(0,0,80,160,COLOR_B);
	
	if ( showMacFlag == 1 )
	{
		ExitDeepSleepMode();
		
		show_information_val( 56, 29 );
		ui_show_version();
		
		EntryDeepSleepMode();
		
	}else{

		DrawUI_Bitmap_Ex(4,16,72,72,BMP_info_1, 0X24BE);
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_info_2, 0XFFFF);
		DrawUI_Bitmap_Ex(4,16,72,72,BMP_info_3, 0X0000);

		DrawUI_Bitmap_Ex(16,116,48,16,bmp_str_info, 0xFFFF);
		
		UI_Draw_Buttom(7,0XFCE0);
	}
}

unsigned long win_info_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0;  showMacFlag = 0;

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
		
		if ( win_ticks > 6 )
		{
			lcd_display(0);
			
			return ( msg_type ^ WINDOW_UPDATE );
		}
           		
		show_information( 28, 20 );
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
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
			win_ticks = 0;
		}

		if ( showMacFlag == 0 )
		{
			showMacFlag = 1;
		}else{
			showMacFlag = 0;
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
		window_pop(&win_current);
		
		return window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0 );
	}	
	
	return 0;
}
