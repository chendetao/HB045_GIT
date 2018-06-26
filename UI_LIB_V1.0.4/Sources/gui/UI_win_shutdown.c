#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_GUI.h"
#include "Task_Motor.h"
#include "LCD_ST7735.h"

#include "string.h"
#include "Window.h"
#include "UserSetting.h"
#include "UI_win_shutdown.h"
#include "UI_Draw.h"
#include "UI_shutdown_icon.h"
#include "bma253.h"
#include "Task_Battery.h"

#define COLOR_B      0X0000
#define COLOR_F      0x1F<<11



#include "UI_comm_icon.h"

const unsigned char power_down[][14] = 
{
{0x08,0x20,0xF8,0x3F,0x08,0x21,0x08,0x01,0x08,0x01,0xF0,0x00,0x00,0x00},/*"P",0*/
{0x00,0x00,0x00,0x1F,0x80,0x31,0x80,0x20,0x80,0x21,0x00,0x1F,0x00,0x00},/*"o",1*/
{0x80,0x00,0x80,0x0F,0xC0,0x3F,0x80,0x07,0x00,0x38,0x80,0x0F,0x80,0x00},/*"w",2*/
{0x00,0x00,0x00,0x1F,0x80,0x22,0x80,0x22,0x80,0x22,0x00,0x13,0x00,0x00},/*"e",3*/
{0x80,0x20,0x80,0x20,0x80,0x3F,0x00,0x21,0x80,0x20,0x80,0x01,0x00,0x00},/*"r",4*/
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*" ",5*/
{0x08,0x20,0xF8,0x3F,0x08,0x20,0x08,0x20,0x10,0x10,0xE0,0x0F,0x00,0x00},/*"D",6*/
{0x00,0x00,0x00,0x1F,0x80,0x31,0x80,0x20,0x80,0x21,0x00,0x1F,0x00,0x00},/*"o",7*/
{0x80,0x00,0x80,0x0F,0xC0,0x3F,0x80,0x07,0x00,0x38,0x80,0x0F,0x80,0x00},/*"w",8*/
{0x80,0x20,0x80,0x3F,0x00,0x21,0x80,0x00,0x80,0x20,0x80,0x3F,0x00,0x20},/*"n",9*/
{0x70,0x00,0x08,0x00,0x08,0x30,0x08,0x37,0x18,0x01,0xF0,0x00,0x00,0x00},/*"?",10*/
};

#define YES 1
#define NO  0

unsigned char in_exit_mode = 0;
unsigned char yes_no = NO;

void UI_Draw_Buttom(int index, int color);

extern void ConnectionTerminate( int enable );
extern unsigned char showLogoFlag;
/**
 * 关机开机回调方法.
 */
void shutdown_cb( int shutdown )
{
    if ( shutdown == 1 )
    {
        /* 关机 */
        
        ConnectionTerminate(1);  // 终止连接
        
        /* 关闭蓝牙广播 */
        
        make_bma250_to_deep_sleep(1);
    }else
    {
        /* 开机 */
        
        /* 关闭蓝牙广播 */    
        
        ConnectionTerminate(0);
        
        make_bma250_to_deep_sleep(0); 
		
		// showLogoFlag = 1;
    }
	
    if ( power.charge_flage == 0 )
    {
        osal_set_event( taskMotorTaskId, TASK_MOTOR_ACTION_DONE_EVT );
    }        
}

void show_shutdown_icon(int x, int y )
{	
	DrawUI_Bitmap_Ex(4,16,72,72,BMP_power_1, 0x1F<<11);
	DrawUI_Bitmap_Ex(4,16,72,72,BMP_power_2, 0xFFFF);

	DrawUI_Bitmap_Ex(16,116,48,16,bmp_str_power, 0xFFFF);

	UI_Draw_Buttom(8,0x1F<<11);	
}

void show_shutdown_comfirm(void)
{
	DrawUI_Bitmap_Ex(4,16,72,72,BMP_power_1, 0x1F<<11);
	DrawUI_Bitmap_Ex(4,16,72,72,BMP_power_2, 0xFFFF);  

    for ( int i = 0; i < 5; i++ )
    {
       DrawUI_Bitmap_Ex( i*7+2, 116, 7, 16, power_down[i],0xFFFF);
    }
    
    for ( int i = 6; i < 10; i++ )
    {
       DrawUI_Bitmap_Ex( i*7-1, 116, 7, 16, power_down[i],0xFFFF);
    }    
    DrawUI_Bitmap_Ex( 10*7+2, 116, 7, 16, power_down[10],0xFFFF);
  
    if ( yes_no == YES )
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

unsigned long win_shutdown_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{	
	if ( msg_type & WINDOW_CREATE )
	{
		win_ticks = 0; in_exit_mode = 0; yes_no = NO;
	
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
 		
		memset(frame_buffer, 0x00, sizeof(frame_buffer));

        if ( in_exit_mode == 0 )
        {
            show_shutdown_icon( 28, 20 );
        }else{
            show_shutdown_comfirm();
        }
        
		sync_frame();
		
		osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 500);
		
		return ( msg_type ^ WINDOW_UPDATE );
	}
	
	if ( msg_type & WINDOW_TOUCH )
	{		
        win_ticks = 0;
        
		if ( config.shutdown == 1 )
		{
			return ( msg_type ^ WINDOW_TOUCH );
		}
		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);			
			win_ticks = 0;
			msg_type |= WINDOW_UPDATE;
		}else{
            if ( in_exit_mode == 0 )
            {         
                window_push(window_get(win_current));
                msg_type |= WINDOW_DISTORY;	
            }else{
                if ( yes_no == NO )
                {
                    yes_no = YES;
                }else{
                    yes_no = NO;
                }
                
                msg_type |= WINDOW_UPDATE;
            }                
		}
		
		return ( msg_type ^ WINDOW_TOUCH );
	}
	
	if ( msg_type & WINDOW_PRESS )
	{	
        win_ticks = 0;
        
        if ( (config.lcdEnable == 0) && (config.shutdown == 0) && ( config.shutdown_no_comfirm == 0) )
        {
            lcd_display(1);
            msg_type |= WINDOW_UPDATE;   
            return ( msg_type ^ WINDOW_PRESS );
        }
        
        if ( config.shutdown_no_comfirm == 0 )
        {
            if ( (in_exit_mode == 0) && (config.shutdown == 0) )
            {
                in_exit_mode = 1;
                return ( msg_type ^ WINDOW_PRESS );
            }
            
            if ( (in_exit_mode == 1) && (config.shutdown == 0) )
            {
                if ( yes_no == NO )
                {
                    in_exit_mode = 0;
                    msg_type |= WINDOW_UPDATE;
                    
                    return ( msg_type ^ WINDOW_PRESS );
                }
            }
        }
        
		if ( config.shutdown == 0 )
		{
			shutdown_cb(1);
			lcd_display(0);
			config.shutdown = 1;
            in_exit_mode = 0; yes_no = NO;
            
			osal_stop_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT );
			osal_clear_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
		}else{
			if ( (power.battery_level >= 1) ||
				(power.charge_flage == 1) )
			{
				shutdown_cb(0);
				config.shutdown = 0;
				
				window_tack_reset();
				window_push(0);   // 开机跳转主页面
				msg_type |= WINDOW_DISTORY;
			}
		}
            
        config.shutdown_no_comfirm = 0;        
		
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
