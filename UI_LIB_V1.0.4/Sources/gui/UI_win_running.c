
//#include "OSAL.h"
//#include "OSAL_Timer.h"

//#include "Task_GUI.h"
//#include "Task_Step.h"
//#include "Task_Store.h"
//#include "LCD_ST7735.h"

//#include "string.h"
//#include "Window.h"
//#include "UserSetting.h"
//#include "UI_Draw.h"
//#include "ClockTick.h"
//#include "UI_hr_icon.h"

//#include "UI_comm_icon.h"
//#include "UI_Comm_Draw.h"
//#include "QR_Utils.h"
//#include "UI_main_icon.h"
//#include "Ui_win_running.h"

//unsigned char running_enable;



//void show_Degree( int x, int y, int w, int h, int level )
//{
//	for ( int j = y; j < y + h; j++ )
//	{
//		for ( int i = x; i < x + w; i++ )
//		{
//			if ( !(i%2) )
//			{
//				setpixel(i, j, 0xFFFF);
//			}
//		}
//	}
//	
//	for ( int j = y; j < y + h; j++ )
//	{
//		for ( int i = x; i < x + w; i++ )
//		{
//			if ( !(i%2) )
//			{
//				setpixel(i, j, 0xFFFF);
//			}
//		}
//	}	
//}

//void show_pedometer(int x, int y )
//{		
//	draw_fill(0,0,80,160,0x1F<<0);
//	
//	DrawUI_Bitmap(0,0,80,80,BMP_pedometer, 0xFFFF<<0,0x1F<<0);
//	
//	show_pedometer_val((80-16*5)/2, 112, pedometer.counter);
//}

//void show_distance_val( int x, int y, int distance )
//{
//    float value;
//    
//    if ( config.unit == 0 )
//    {
//        value = distance / 1000.0;
//    }else{
//        value = distance * 0.0006214;
//    }
//    int vh = (int)value;
//    int vl = (int)((value - vh) *100);
//    int v2,v3;
//    v2 = vl/10;
//    v3 = vl%10;
//    
//    if ( v3 >= 5 )
//    {
//        v2 += 1;
//        if ( v2 > 9 )
//        {
//            v2 = 0;
//            value += 1;
//        }
//    }	
//    
//    vl = v2;
//    
//    if ( vh > 9 )
//    {
//		DrawUI_Bitmap(x, y, 8,18,Font_8x16_Other[(vh/10)%10],0xffE0,0X1f<<0);
//		x += 7;
//    }
//    
//    DrawUI_Bitmap(x, y, 8,16, Font_8x16_Other[vh%10],0xffE0,0X1f<<0);
//    x += 8;
//    DrawUI_Bitmap(x, y, 8,16, Font_8x16_Other[10],0xffE0,0X1f<<0);
//    x += 4;
//    DrawUI_Bitmap(x, y, 8,16, Font_8x16_Other[vl],0xffE0,0X1f<<0);	
//}

//void DrawCaloriesValue( int x, int y, int value )
//{
//    int count;
//    
//    if ( value > 99999 ){ value = 99999; }
//    
//    count = value;
//    
//    int width = 0;
//    do
//    {
//      width += 8;
//      count /= 10;
//    }while( count > 0 );
//    
//    // x+((64-x) - width)/2; // Align center from x
//    int sx = x; // Align left 
//    
//    int ex = sx + width - 8;
//    do
//    {
//      DrawUI_Bitmap(ex, y, 8, 16, Font_8x16_Other[value%10],0xffE0,0X1f<<0);	
//      value /= 10;
//      ex -= 8;  
//    }while( value > 0 );  
//}

//void show_pedometer_timer( int x, int y )
//{
//	draw_fill(0,0,80,160,0x1F<<0);	
//	
//	show_pedometer_val((80-16*5)/2, 24, pedometer.counter - config.train_running_pedometer);
//	show_distance_val(16,80, pedometer.distance-config.train_running_distance);
//	DrawCaloriesValue(16, 14*8,pedometer.calorie-config.train_running_calorie);
//	
//	DrawUI_Bitmap(56, 84, 20, 16, bmp_km,0xffE0,0X1f<<0);     		// ICON
//	DrawUI_Bitmap( 0, 80, 16, 16, bmp_distance,0xffE0,0X1f<<0);     // val
//	
//	DrawUI_Bitmap(56-4, 14*8+4, 24, 16, bmp_kc,0xffE0,0X1f<<0);     // ICON
//	DrawUI_Bitmap( 0, 14*8, 16, 16, bmp_caluire,0xffE0,0X1f<<0);    // val
//	
//	// 这里应该显示一个运动定时器吧？ 但是UI要求显示日期
//	
//	DrawUI_Bitmap( 0, 144, 8, 16, Font_8x16_Other[2], 0xffE0,0X1f<<0);
//	DrawUI_Bitmap( 8, 144, 8, 16, Font_8x16_Other[0], 0xffE0,0X1f<<0);
//	DrawUI_Bitmap(16, 144, 8, 16, Font_8x16_Other[TimerYY()/10], 0xffE0,0X1f<<0);
//	DrawUI_Bitmap(24, 144, 8, 16, Font_8x16_Other[TimerYY()%10], 0xffE0,0X1f<<0);
//	DrawUI_Bitmap(32, 144, 8, 16, Font_8x16_Other[11], 0xffE0,0X1f<<0);	
//	DrawUI_Bitmap(40, 144, 8, 16, Font_8x16_Other[TimerMM()/10], 0xffE0,0X1f<<0);
//	DrawUI_Bitmap(48, 144, 8, 16, Font_8x16_Other[TimerMM()%10], 0xffE0,0X1f<<0);
//	DrawUI_Bitmap(56, 144, 8, 16, Font_8x16_Other[11], 0xffE0,0X1f<<0);		
//	DrawUI_Bitmap(64, 144, 8, 16, Font_8x16_Other[TimerDD()/10], 0xffE0,0X1f<<0);
//	DrawUI_Bitmap(72, 144, 8, 16, Font_8x16_Other[TimerDD()%10], 0xffE0,0X1f<<0);
//}

//unsigned long win_running_proc(unsigned long wid, unsigned long msg_type, 
//                            unsigned long msg_param, unsigned long *p)
//{
//	if ( msg_type & WINDOW_CREATE )
//	{
//		win_ticks = 0;  running_enable = 0;

//		if ( config.lcdEnable == 0 )
//		{
//			lcd_display(1);
//		}		

//		osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
//		
//		return ( msg_type ^ WINDOW_CREATE );
//	}
//	
//	if ( msg_type & WINDOW_UPDATE )
//	{
//		win_ticks++;
//		
//		if ( win_ticks > 10 )
//		{
//			lcd_display(0);
//			
//			return ( msg_type ^ WINDOW_UPDATE );
//		}
//		
//		memset( frame_buffer, 0, sizeof(frame_buffer));

//		if ( running_enable == 0 )
//		{
//			show_pedometer( 28, 20 );
//		}else{
//			show_pedometer_timer( 28, 20 );
//		}

//		sync_frame();
//		
//		osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 500);		
//				
//		return ( msg_type ^ WINDOW_UPDATE );
//	}
//	
//	if ( msg_type & WINDOW_TOUCH )
//	{		
//		if ( config.lcdEnable == 0 )
//		{
//			lcd_display(1);
//			win_ticks = 0;
//			msg_type |= WINDOW_UPDATE;
//		}else{
//			
//			if ( running_enable == 0 )
//			{
//				msg_type |= WINDOW_DISTORY;				
//			}
//		}
//		
//		return ( msg_type ^ WINDOW_TOUCH );
//	}
//	
//	if ( msg_type & WINDOW_PRESS )
//	{	
//		win_ticks = 0;
//		
//		if ( config.lcdEnable == 0 )
//		{
//			lcd_display(1);
//			
//			msg_type |= WINDOW_UPDATE;
//			
//			return ( msg_type ^ WINDOW_PRESS );
//		} else {
//			
//			if ( running_enable == 0 )
//			{
//				running_enable = 1;
//				
//				config.train_running_utc  = getUTC();
//				config.train_running_distance  = pedometer.distance;
//				config.train_running_pedometer = pedometer.counter;
//				config.train_running_calorie   = pedometer.calorie;
//				
//			} else
//			{
//				running_enable = 0;
//				
//				osal_set_event( taskStoreTaskId, TASK_STORE_SAVE_RUNNING_EVT );
//			}
//		}
//		
//		return ( msg_type ^ WINDOW_PRESS );
//	}
//	
//	if ( msg_type & WINDOW_NOTIFY )
//	{   
//        /* 当前窗体入栈 */
//        window_push( win_current );
//        
//        /* 通知界面作为当前界面 */
//        win_current = NOTIFY_WIN_ID_GET();
//        
//        /* 调用窗体回调函数以便创建窗体  */
//        window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0);
//		
//		return ( msg_type ^ WINDOW_NOTIFY );
//	}	
//	
//	if ( msg_type & WINDOW_DISTORY )
//	{
//		window_pop(&win_current);
//		window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0 );
//		
//		return ( msg_type ^ WINDOW_DISTORY );
//	}	
//	
//	return 0;
//}
