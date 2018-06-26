
#include "string.h"
#include "Window.h"

#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_GUI.h"

#include "LCD_ST7735.h"
#include "QR_Utils.h"
#include "Ancs.h"
#include "UserSetting.h"
#include "UI_Draw.h"
#include "btprotocol.h"

#include "UI_win_notify.h"
#include "Window.h"
#include "UI_notify_icon.h"
#include "MsgQueue.h"
#include "SPI.h"

void NOTICE_ADD(unsigned char notify_id)
{
    mq.notify_id = notify_id;
    osal_set_event(taskGUITaskId, TASK_GUI_NOTIFY_EVT);
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "MsgQueue.h"
#include "Alarm.h"
#include "HFMA2Ylib.h"

unsigned char DZ[32] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x18,};

 /**
 * uint8 LengthCount( uint8 *pVal, uint8 length);
 * @bref 计算字符(Ascii、GBK)所占宽度.
 * @pVal Buffer pointer the store the chars.
 * @lenght length of buffer
 * return 所有字符宽度的像素值.
 */
unsigned int LengthCount( unsigned char *pVal, int length)
{
    unsigned int x = 0;
    
    for ( int j = 0; j < length; j += 2 )
    {
        if ( pVal[j] == 0x0 )
        {
            x += 8;
        }else{
            x += 16;
        }
    }
             
    return x;
}

void DoRect16x16( int cx, int cy, const unsigned char pic[32])
{   
    unsigned char temp;
    
    for ( int k = 0; k < 16 ; k++)
    {        
		temp = pic[k+0];	
		for ( int i = 0; i < 8; i++ )
		{
			if ( (temp >> i) & 0x1 )
			set_pixel(cx, cy+i+0, 0xFFFF);
		}
		
		temp = pic[k+16];
		for ( int i = 0; i < 8; i++ )
		{
			if ( (temp >> i) & 0x1 )
			set_pixel(cx, cy+i+8, 0xFFFF);
		}		
      
        cx++;
    }
}

void DoRect8x16( int cx, int cy, const unsigned char pic[32])
{   
    unsigned char temp;
    
    for( int k = 0; k < 8 ; k++)
    {        
		temp = pic[k+0];	
		for ( int i = 0; i < 8; i++ )
		{
			if ( (temp >> i) & 0x1 )
			set_pixel(cx, cy+i+0, 0xFFFF);
		}
		
		temp = pic[k+8];
		for ( int i = 0; i < 8; i++ )
		{
			if ( (temp >> i) & 0x1 )
			set_pixel(cx, cy+i+8, 0xFFFF);
		}		
		
        cx++;
    }
}

/**
 * static void DrawTitle(void);
 *
 * Note : Show the notification Message Title( sender )
 *
 */
static void DrawTitle(void)
{
    /*计算出中心对称显示时的起始坐标 */
    int x,y,len;
	unsigned short gbk;
	unsigned short unicode;
	
	ExitDeepSleepMode();	
	
    len = LengthCount(mq.title, mq.title_len);
    if ( len <= LCD_W )
    {
      x = (LCD_W - len)/2;
      y = (LCD_H-16)/2;
    }else
    {
      x = 0; y = (LCD_H-16)/2;
    }
    
    for ( int j = 0; j < mq.title_len; j += 2 )
    {
		if ( ((mq.title[j] == 0x20) && (mq.title[j+1] == 0x2D)) 
			|| ( (mq.title[j] == 0x20) && (mq.title[j+1] == 0x2C))
            || ( (mq.title[j] == 0x20) && (mq.title[j+1] == 0x0E))  
            || ( (mq.title[j] == 0x26) && (mq.title[j+1] == 0x0E))        )
		{
			continue;
		}			
		
        if ( mq.title[j] == 0x0 )
        {
			// if ( mq.title[j+1] != 0XA && mq.title[j+1] != 0xD ){  // \n
			if ( mq.title[j+1] < 0X20 || mq.title[j+1] > 0x7E ){
				continue;
			}
			{
				ASCII_GetData(mq.title[j+1],ASCII_8X16,DZ);
				DoRect8x16(x, y, DZ);
				x += 8; len -= 8;
			}
        }else{
            unicode = (unsigned short)(mq.title[j]<<8)
				+ (unsigned short)mq.title[j+1];
			
			// add
			if (  (unicode>=0x2E80 && unicode<=0x2FDF) ||
				( (unicode>=0x3400) && (unicode <= 0x4DBE)) ||
			    ( (unicode>=0x4E00) && (unicode<=0x9FFF) ) )
			{
				// 中文区
				gbk = U2G(unicode );
				hzbmp16( SEL_GB, gbk ,0, 16, DZ ); 
				
				DoRect16x16(x,y,DZ);
				x += 16;
				
			}else		
			if ( (unicode >= 0x1100 && unicode <= 0x11FF) ||
                (unicode>=0x3130 && unicode <= 0x318F) ||
                  (unicode >= 0xAC00 && unicode <= 0xD7AF) )
            {
				// 韩文区
				unsigned short ksc;
				
				ksc = U2K(unicode);
			    hzbmp16( SEL_KSC, ksc ,0, 16, DZ );
				
				DoRect16x16(x,y,DZ);
				x += 16;
				
            }else if ( (unicode>=0x3040 && unicode<=0x30FF) ||
				( (unicode>=0x31F0) && (unicode <= 0x31FF)) ){
				// 日文区
				unsigned short jis;
					
				jis = U2J(unicode);
			    hzbmp16( SEL_JIS, jis ,0, 16, DZ );
				
				DoRect16x16(x,y,DZ);
				x += 16;	
				mq.pageLength += 2;
			}else{
				// 其他外国文字
				if ( (unicode>=0x0080 && unicode<=0x02AF) ||
					( (unicode>=0x1E00) && (unicode<=0x1EFF)) )
				{
					// 拉丁文
					// 包含但不限于：德语、法语、西班牙语、葡萄牙语
					//               意大利语
					LATIN_GetData(unicode,DZ);  // 不等宽
					DoRect8x16(x,y,DZ);
					x += 8;
					
				}else if ( (unicode >= 0x0400) && (unicode <= 0x052F) )
				{
					// 西里尔文
					// 包含但不限于：俄语
					CYRILLIC_GetData(unicode,DZ); // // 不等宽
					DoRect8x16(x,y,DZ);
					x += 8;
					
				}else
				{
					// 按照汉字处理
					gbk = U2G(unicode );
					hzbmp16( SEL_GB, gbk ,0, 16, DZ ); 
					
					DoRect16x16(x,y,DZ);
					x += 16;
				}
            }
			// end
        }
		
		if ( (j+2) >= mq.title_len ) { break; }
        
        if ( mq.title[j+2] == 0x0 ){
            if ( (x+8) > LCD_W ) 
			{ 
				y += 16; 
				if ( len <= LCD_W )
					x = (96-len)/2;
			}
        }else{
            if ( (x+16) > LCD_W )
			{ 
				y += 16;
				if ( len <= LCD_W )  
					x = (LCD_W-len)/2; 
			}
        }        
    } 
	
	EntryDeepSleepMode();
}

/**
 * static void DrawBody( int line );
 *
 * Note : Show the notification Message Body( Content )
 *
 */
static void DrawBody( int x, int line )
{
    int y = line*16;
	unsigned short gbk;
	unsigned short unicode;
	
	ExitDeepSleepMode();	
	
	mq.pageLength = 0;
        
    for ( int i = mq.offset; i < mq.body_len; i += 2)
    {		
		if ( ((mq.body[i] == 0x20) && (mq.body[i+1] == 0x2D)) 
			|| ( (mq.body[i] == 0x20) && (mq.body[i+1] == 0x2C))
            || ( (mq.body[i] == 0x20) && (mq.body[i+1] == 0x0E))
            || ( (mq.body[i] == 0x26) && (mq.body[i+1] == 0x0E))
            || ( (mq.body[i] == 0x20) && (mq.body[i+1] == 0x2A))     
            || ( (mq.body[i] == 0xF3) && (mq.body[i+1] == 0xA4)) )
		{
			continue;
		} 
        
        if ( mq.body[i] == 0x0 /* && (mq.body[i+1] < 0x80)*/ )
        {
			// if ( (mq.body[i+1] != 0xA) && (mq.body[i+1] != 0xD) )
			if ( mq.body[i+1] < 0X20 || mq.body[i+1] > 0x7E ){
				continue;
			}
			
			{
				ASCII_GetData(mq.body[i+1],ASCII_8X16,DZ);
				DoRect8x16(x, y, DZ);
				x += 8;
				mq.pageLength += 2;
			}
        }else{
            unicode = (unsigned short)(mq.body[i]<<8) + mq.body[i+1];
            
			// add
			if (  (unicode>=0x2E80 && unicode<=0x2FDF) ||
				( (unicode>=0x3400) && (unicode <= 0x4DBE)) ||
			    ( (unicode>=0x4E00) && (unicode<=0x9FFF) ) )
			{
				// 中文区
				gbk = U2G(unicode );
				hzbmp16( SEL_GB, gbk ,0, 16, DZ ); 
				
				DoRect16x16(x,y,DZ);
				x += 16;
				mq.pageLength += 2;
			}else		
			if ( (unicode >= 0x1100 && unicode <= 0x11FF) ||
                (unicode>=0x3130 && unicode <= 0x318F) ||
                  (unicode >= 0xAC00 && unicode <= 0xD7AF) )
            {
				// 韩文区
				unsigned short ksc;
				
				ksc = U2K(unicode);
			    hzbmp16( SEL_KSC, ksc ,0, 16, DZ );
				
				DoRect16x16(x,y,DZ);
				x += 16;
				mq.pageLength += 2;
            }else if ( (unicode>=0x3040 && unicode<=0x30FF) ||
				( (unicode>=0x31F0) && (unicode <= 0x31FF)) ){
				// 日文区
				unsigned short jis;
					
				jis = U2J(unicode);
			    hzbmp16( SEL_JIS, jis ,0, 16, DZ );
				
				DoRect16x16(x,y,DZ);
				x += 16;	
				mq.pageLength += 2;
			}else{
				// 其他外国文字
				if ( (unicode>=0x0080 && unicode<=0x02AF) ||
					( (unicode>=0x1E00) && (unicode<=0x1EFF)) )
				{
					// 拉丁文
					// 包含但不限于：德语、法语、西班牙语、葡萄牙语
					//               意大利语
					LATIN_GetData(unicode,DZ);  // 不等宽
					DoRect8x16(x,y,DZ);
					x += 8;
					mq.pageLength += 2;
					
				}else if ( (unicode >= 0x0400) && (unicode <= 0x052F) )
				{
					// 西里尔文
					// 包含但不限于：俄语
					CYRILLIC_GetData(unicode,DZ); // // 不等宽
					DoRect8x16(x,y,DZ);
					x += 8;
					mq.pageLength += 2;
					
				}else
				{
					// 按照汉字处理
					gbk = U2G(unicode );
					hzbmp16( SEL_GB, gbk ,0, 16, DZ ); 
					
					DoRect16x16(x,y,DZ);
					x += 16;
					mq.pageLength += 2;
				}
            }
			// end			
        }

		if ( (i+2) >= mq.body_len ) { break; }
		
        if ( mq.body[i+2] == 0x0 ){
            if ( x+8 > LCD_W ) 
			{ 
				y += 16; x = 0; 			  
				if ( y >= LCD_H )
				{
					break;
				}
			} 
        }else{
            if ( (x+16) > LCD_W ) 
			{ 
				y += 16; x = 0;
				if ( y >= LCD_H )
				{
					break;
				}
			}
        }
    }   

	EntryDeepSleepMode();	
}

#include "version.h"

void show_ui_version(void)
{
	mq.title_len = 12;
	
	mq.title[0] = 0x00;
	mq.title[1] = 0x56;
	mq.title[2] = 0x00;
	mq.title[3] = '0' + VERSION_MAJOR;
	mq.title[4] = 0x00;
	mq.title[5] = '.';
	mq.title[6] = 0x00;	
	mq.title[7] = '0' + VERSION_MINOR;
	mq.title[8] = 0x00;
	mq.title[9] = '.';
	mq.title[10] = 0x00;
	mq.title[11] = '0' + VERSION_REVISON;
		
	DrawTitle();
}

bool font_is_ok(void)
{
	unsigned int GB_CODE;
	
	ExitDeepSleepMode();
	
	GB_CODE = U2G(0x554a);
	
	EntryDeepSleepMode();
	
	if ( GB_CODE == 0xb0a1 )
	{
		 return true;
	}
	return false;
}

#include "UI_notify_icon.h"

unsigned char ccflag = 0;
///////////////////////////////////通知界面(子界面：来电) //////////////////////

void ui_calling_notify( int msg_id, unsigned long arg)
{
    memset(frame_buffer, 0, sizeof(frame_buffer));

	if ( (++ccflag %2 ) == 0){
		DrawUI_Bitmap((80-40)/2,(160-40)/2,40,40,BMP_call,BMP_CALL_RGB_F,BMP_CALL_RGB_B);	
	}else
	{    
		if ( mq.title_len != 0 )
		{
			DrawTitle();
		}else if ( !(ccflag %2) )
		{
			DrawUI_Bitmap((80-40)/2,(160-40)/2,40,40,BMP_call,BMP_CALL_RGB_F,BMP_CALL_RGB_B);	
		}    
	}
	
    sync_frame();
}

void ui_erase_calling_notify(int msg_id, unsigned long arg)
{
    memset(frame_buffer, 0, sizeof(frame_buffer));
	
    sync_frame();
}

///////////////////////////////////通知界面(子界面:消息) //////////////////////

void DrawIcon(void)
{
	if ( mq.type == NOTIFY_TYPE_CALL )
	{
		DrawUI_Bitmap((80-40)/2,(160-40)/2,40,40,BMP_call,BMP_CALL_RGB_F,BMP_CALL_RGB_B);	
	}else if ( mq.type == NOTIFY_TYPE_MSG )
	{ 
		DrawUI_Bitmap((80-49)/2,(160-32)/2,49,32,BMP_msg,BMP_MSG_RGB_F,BMP_MSG_RGB_B);
	}else if ( mq.type == NOTIFY_TYPE_QQ )
	{
		DrawUI_Bitmap((80-41)/2,(160-48)/2,41,48,BMP_qq,BMP_QQ_RGB_F,BMP_QQ_RGB_B);
	}else if ( mq.type == NOTIFY_TYPE_WX )
	{
		DrawUI_Bitmap((80-58)/2,(160-48)/2,58,48,BMP_wx, BMP_WX_RGB_F,BMP_WX_RGB_B);
	}else if ( mq.type == NOTIFY_TYPE_WHATSAPP )
	{                                                
		DrawUI_Bitmap(8,(160-64)/2,64,64,BMP_wa, BMP_WA_RGB_F,BMP_WA_RGB_B);
	}else if ( mq.type == NOTIFY_TYPE_FACEBOOK )
	{
		DrawUI_Bitmap((80-30)/2,(160-56)/2,30,56,BMP_fb, BMP_FB_RGB_F,BMP_FB_RGB_B);
	}else if ( mq.type == NOTIFY_TYPE_LINE )
	{
		draw_bitmap( (80-64)/2, (160-64)/2, 64, 64, gImage_line);
	}else if ( mq.type == NOTIFY_TYPE_TT ){
        DrawUI_Bitmap_Exx((80-54)/2,(160-48)/2,54,48,BMP_Twitter, 0x057E);
    }else if ( mq.type == NOTIFY_TYPE_INS ){
        DrawUI_Bitmap_Exx((80-48)/2,(160-48)/2,48,48,BMP_ins_1, 0XD8CF);
        DrawUI_Bitmap_Exx((80-33)/2,(160-40)/2,33,40,BMP_ins_2, 0xffff);
    }else if ( mq.type == NOTIFY_TYPE_SKYPE ){
        DrawUI_Bitmap_Exx((80-50)/2,(160-48)/2,50,48,BMP_skype_1, 0x057E);
        DrawUI_Bitmap_Exx((80-26)/2,(160-32)/2,26,32,BMP_skype_2, 0xFFFF);        
    }
}

void ui_message_notify( int msg_id, unsigned long arg)
{
    memset(frame_buffer, 0, sizeof(frame_buffer));
	
	if ( !( arg & 0x1) )
	{
		if ( (++ccflag %2 ) == 0){
			DrawIcon();
		}else
		{    
			if ( mq.title_len != 0 )
			{
				DrawTitle();
			}else if ( !(ccflag %2) )
			{
				DrawIcon();
			}    
		}
	
	}else{
		DrawBody(0,0);
	}
	
    sync_frame();
}

void ui_erase_message_notify(int msg_id, unsigned long arg)
{
    memset(frame_buffer, 0, sizeof(frame_buffer));
    sync_frame();
}

///////////////////////// APP 消息通知 /////////////////////////////////////////

void ui_app_notify( int msg_id, unsigned long arg )
{
    memset(frame_buffer, 0, sizeof(frame_buffer));
	
	if ( !( arg & 0x1) )
	{
		if ( (++ccflag %2 ) == 0){
			DrawIcon();
		}else
		{    
			if ( mq.title_len != 0 )
			{
				if ( ( (mq.push_by_ios == 1 
					&& mq.type == NOTIFY_TYPE_WHATSAPP ) ) 
                || ( (mq.type == NOTIFY_TYPE_WX) && (ancs_ancs_flag == 1)) 
                || ( (mq.push_by_ios == 1) && (mq.type == NOTIFY_TYPE_LINE ) ) )
				{
					DrawTitle();
				}
			}else if ( !(ccflag %2) )
			{
				DrawIcon();
			}    
		}
	
	}else{

		DrawBody(0,0);
	}
	
    sync_frame();  
}

void ui_earse_app_notify( int msg_id, unsigned long arg ){}

///////////////////////////////////通知界面(子界面：闹钟) //////////////////////

#include "UI_function_icon.h"
#include "Ui_alarm_icon.h"	
#include "UI_comm_icon.h"
#include "UI_win_alarm.h"
#include "ClockTick.h"
	
static void DrawAlarmTimer(int x, int y, int hour, int minute )
{	
	 static unsigned char showFlag = 0;
	
	 if ( showFlag++ % 2 ){
		
        #if 0
		DrawUI_Bitmap(0,24,80,80,BMP_function_alarm,BMP_FUNCTION_ALARM_RGB_F,BMP_FUNCTION_ALARM_RGB_B);	
		#else
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x6000);             
        #endif
	 }else{

		DrawUI_Bitmap_Ex(13,68,11,24,Font_11x24[hour/10], 0xFFFF);	
		DrawUI_Bitmap_Ex(24,68,11,24,Font_11x24[hour%10], 0xFFFF);	
		DrawUI_Bitmap_Ex(35,68,11,24,Font_11x24[11], 0xFFFF);	
		DrawUI_Bitmap_Ex(45,68,11,24,Font_11x24[minute/10], 0xFFFF);	
		DrawUI_Bitmap_Ex(56,68,11,24,Font_11x24[minute%10], 0xFFFF);		 		 
	 }
}	
	
void ui_alarm_notify( int msg_id, unsigned long arg )
{
    memset(frame_buffer, 0, sizeof(frame_buffer));
    
    /* 绘制当前闹钟而不是下一个闹钟时间 */
    		
	DrawAlarmTimer((80-53)/2,(160-42)/2, alarm.g_Ahour_cur, alarm.g_Amin_cur);
	
    sync_frame();
}

void ui_erase_alarm_notify( int msg_id, unsigned long arg ){}

///////////////////////////////// 通知界面（子界面：手机找设备）////////////////


#include "UI_findphone_icon.h"	
	
void ui_find_me_notify( int msg_id, unsigned long arg )
{   
    memset(frame_buffer, 0, sizeof(frame_buffer));
    
	if ( config.showFlag++ % 2 ){
		DrawUI_Bitmap_Ex(0,(160-80)/2,80,80,BMP_phone,BMP_FUNCTION_FINDPHONE_RGB_F);
	}
	
    sync_frame();
}

void ui_erase_find_me_notify( int msg_id, unsigned long arg ){}

////////////////// 通知子界面 (蓝牙断开) ///////////////////////////////////////
void ui_ble_notify( int msg_id, unsigned long arg )
{  
    memset(frame_buffer, 0, sizeof(frame_buffer));

	if ( config.showFlag++ & 0x1 )
	{
		DrawUI_Bitmap_Ex(8,(160-64)/2,64,64,BMP_ble, BMP_BLE_RGB_F);
	}
		
    sync_frame();
}

void ui_erase_ble_notify( int msg_id, unsigned long arg ){}

////////////////// 通知子界面 (久坐提醒) ///////////////////////////////////////
void ui_sedentary_notify( int msg_id, unsigned long arg )
{  
    memset(frame_buffer, 0, sizeof(frame_buffer));
	
	if ( config.showFlag++ & 0x1 )
	{	
		DrawUI_Bitmap_Ex(8,(160-64)/2,64,64,BMP_sedentary, BMP_SEDENTARY_RGB_F);
    }
	
    sync_frame();
}

void ui_erase_sedentary_notify( int msg_id, unsigned long arg ){}
    
    
#include "jpeg_decode.h"
#include "UI_sport_icon.h"
#include "Task_UITraviolet.h"
    
extern const unsigned char ui_jpeg_uv[];    
    
////////////////// 通知子界面 (紫外线超标提醒) ///////////////////////////////////////
void ui_uit_notify( int msg_id, unsigned long arg )
{  
    memset(frame_buffer, 0, sizeof(frame_buffer));
	
	if ( config.showFlag++ & 0x1 )
	{	
        DrawUI_JPEG((80-36)/2,20,ui_jpeg_uv);
		DrawUI_JPEG( 17, 57, jpeg_ui_uit_notify );
        int lvl = UIT_i[0];
        
        DrawUI_Bitmap_Exx( (80-11*2)/2,(160-18)/2,11, 24, Font_11x18[lvl/10], 0x0000);
        DrawUI_Bitmap_Exx( (80-11*2)/2+11,(160-18)/2,11, 24, Font_11x18[lvl%10], 0x0000);        
    }
	
    sync_frame();
}

void ui_erase_uit_notify( int msg_id, unsigned long arg ){}    
    
////////////////// 通知子界面 (喝水提醒) ///////////////////////////////////////
void ui_drink_notify( int msg_id, unsigned long arg )
{  
    memset(frame_buffer, 0, sizeof(frame_buffer));
	
	if ( config.showFlag++ & 0x1 )
	{	
        DrawUI_Bitmap_Exx( (80-80)/2,(160-80)/2,80,80, bmp_icon_drink_1, 0xFFFF);
        DrawUI_Bitmap_Exx( (80-80)/2,(160-80)/2,80,80, bmp_icon_drink_2, 0xFFFF);        
    }
	
    sync_frame();
}

void ui_erase_drink_notify( int msg_id, unsigned long arg ){}  

////////////////// 通知子界面 (吃药线超标提醒) ///////////////////////////////////////
void ui_medicine_notify( int msg_id, unsigned long arg )
{  
    memset(frame_buffer, 0, sizeof(frame_buffer));
	
	if ( config.showFlag++ & 0x1 )
	{	
        DrawUI_Bitmap_Exx( (80-80)/2,(160-80)/2,80,80, bmp_icon_medicine, 0xFFFF);      
    }
	
    sync_frame();
}

void ui_erase_medicine_notify( int msg_id, unsigned long arg ){}      
	
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#define NOTIFY_ID_TOINDEX(id) ((id)-1)

/* 根据通知类型的不同使用不同的提醒时长 */
static const int notify_max[10] =
{
    20, /* calling 10s */
    6,  /* MESSAGE EMAIL 3s */
    4,  /* APP_MSG 2s */
    30, /* ALARM 15s */
    14, /* FINDME 7s */   
    4,  /* BLE Disconnnected 8s */
    8,  /* Sedentary notification 4s */ 
    10, /* uit notification 5s */ 
    30, /* Drinking water */
    30, /* Medicine water */    
};

typedef void (*notify_cb)(int msg_id, unsigned long arg);
struct notify
{
    notify_cb update;
    notify_cb close;
};

const struct notify ncb[10] = 
{
  [0] = {.update = ui_calling_notify,.close= ui_erase_calling_notify,},
  [1] = {.update = ui_message_notify,.close= ui_erase_message_notify,},
  [2] = {.update = ui_app_notify,.close = ui_earse_app_notify,},
  [3] = {.update = ui_alarm_notify,.close= ui_erase_alarm_notify,},
  [4] = {.update = ui_find_me_notify,.close= ui_erase_find_me_notify,},
  [5] = {.update = ui_ble_notify,.close= ui_erase_ble_notify,},
  [6] = {.update = ui_sedentary_notify,.close= ui_erase_sedentary_notify,},  
  [7] = {.update = ui_uit_notify, .close = ui_erase_uit_notify,},  
  [8] = {.update = ui_drink_notify, .close = ui_erase_drink_notify,},
  [9] = {.update = ui_medicine_notify, .close = ui_erase_medicine_notify,},  
};

#include "LCD_ST7735.h"
#include "Task_Motor.h"
#include "Task_Finder.h"

unsigned char hangup_cmd[20] = {0xFC,0x08,0x03,00};

extern void LCD_Clear(void);

/***
 * 通知窗口消息处理函数
 */
unsigned long win_notify_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{
    static unsigned char notify_id, flag = 0;
    
    /**
     * 创建窗体：绘制窗体本身的UI元素
     */
    if ( msg_type & WINDOW_CREATE )
    {  
		win_ticks = 0; flag = 0;
        
        notify_id = mq.notify_id;
		
		if ( mq.notify_id == NOTIFY_ID_APP_MSG )
		{
			osal_set_event( taskMotorTaskId, TASK_MOTOR_ACTION_APP_MSG_EVT);
			
			return ( msg_type ^ WINDOW_CREATE );
		}
		
		if ( config.lcdEnable == 0 ){
			lcd_display(1);
		}		
		
		osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
                
        return ( msg_type ^ WINDOW_CREATE );
    }
    
    /**
     * 更新窗体：根据参数更新整个窗体或者某些区域
     */
    if ( msg_type & WINDOW_UPDATE )
    {                
        if ( notify_id == NOTIFY_ID_INVALID )
        {
            msg_type |= WINDOW_DISTORY; // 结束被窗体，从窗体栈恢复上个窗体
            return ( msg_type ^ WINDOW_UPDATE );
        }
		
		if ( config.lcdEnable == 0 ){
			lcd_display(1);
		}		
        
        if ( win_ticks++ < notify_max[notify_id-1] )
        {
			// 要调试来电通知，需要注释掉下面的#If语句;
			// #warning "test code enable ........."
			#if  1
			if ( notify_id == NOFIFY_ID_CALLING && ble.isConnected == 0)
			{
				// 来电提醒时间较长，直到挂断时才结束,若蓝牙在提醒过程中断开,
				// 则提醒结束,避免无法退出提醒.
				win_ticks = notify_max[notify_id-1];
				osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 1);
				return ( msg_type ^ WINDOW_UPDATE );
			}
			#endif
			
            ncb[NOTIFY_ID_TOINDEX(notify_id)].update(notify_id, flag);
           
            if ( flag == 0 )
            {
                if ( (win_ticks % 2 ) == 0 ) 
                {
                    if ( notify_id != NOFIFY_ID_CALLING 
						&& notify_id != NOTIFY_ID_EMAIL
						&& notify_id != NOTIFY_ID_APP_MSG
						&& notify_id != NOTIFY_ID_ALARM
						&& notify_id != NOTIFY_ID_BLE )
                    {   /* lcd_display(0); LCD_Clear(); */ }
                    // MOTOR_ENABLE(1);
					if ( notify_id != NOTIFY_ID_APP_MSG ){
						osal_set_event( taskMotorTaskId, TASK_MOTOR_ACTION_NOTIFY_EVT);
					}
                }else {
                    // MOTOR_ENABLE(0);
                    if ( notify_id != NOFIFY_ID_CALLING 
						&& notify_id != NOTIFY_ID_EMAIL
						&& notify_id != NOTIFY_ID_APP_MSG
					    && notify_id != NOTIFY_ID_ALARM 
					    && notify_id != NOTIFY_ID_BLE )
                    {  /* lcd_display(1);*/ }
                }
            }
			osal_start_timerEx( taskGUITaskId, TASK_GUI_UPDATE_EVT, 500);
        }else{
            
            /* 本轮消息通知结束，查看是否有下一轮 */
            
            /** 一轮通知结束,查看是否有下一通知,如果有，则清除当前变量状态,退出当前
             *  通知,然后返回上一个界面，由上一个界面再进入通知界面
             * 否则，停留在当前通知界面
             */
            mq.notify_id = 0;   
           
            {
				if ( !((flag>>7) & 0x1) )
				{
					win_ticks = 0;
					osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
				
					msg_type |= WINDOW_TOUCH;
				} else
				{
					ncb[NOTIFY_ID_TOINDEX(notify_id)].close(notify_id, 0);
					lcd_display(0);					
				}
			}				
            
            MOTOR_ENABLE(0);
        }
        
        return ( msg_type ^ WINDOW_UPDATE );
    }
    
    /**
     * Touch窗体
     */
    if ( msg_type & WINDOW_TOUCH )
    {
        /* 确保马达停止震动 */
            
        MOTOR_ENABLE(0);  
        
        if ( win_ticks < notify_max[notify_id-1] ) /** 提醒过程中触摸按键 */
        {
            if ( (notify_id == NOTIFY_ID_EMAIL) || (notify_id == NOTIFY_ID_APP_MSG)
				|| (notify_id == NOFIFY_ID_CALLING) )
            {				
				// 有Title或者Body的通知(来电、短信、QQ、微信)：
                if ( config.lcdEnable == 0 ){ 
					/*lcd_display(1);*/ win_ticks = 0;
					osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
					return ( msg_type ^ WINDOW_TOUCH );
				}
                // 显示内容
                if ( ((flag >>7) & 0x1 ) != 1 )
                {
                  if ( (mq.body_len > 0) && (notify_id != NOFIFY_ID_CALLING)){
                    flag = (0x1<<7) | 0x1;  win_ticks = 0;
                  }else{  // 无内容：退出提醒
                    mq.notify_id = 0;
                    msg_type |= WINDOW_DISTORY; // 结束被窗体，从窗体栈恢复上个窗体                     
                  }
                }else{   
                  
                  if ( (mq.body_len == 0) || (notify_id == NOFIFY_ID_CALLING ) )
                  {
                    mq.notify_id = 0;
                    msg_type |= WINDOW_DISTORY; // 结束被窗体，从窗体栈恢复上个窗体                     
                  }
                  
                  if (GET_MESSAGE(msg_param) == WIN_MSG_KEY_LONG_TOUCH ){
                    mq.notify_id = 0;
                    msg_type |= WINDOW_DISTORY; // 结束被窗体，从窗体栈恢复上个窗体      
                  }else
                  {
					  /* 继续更新屏幕,显示内容 */
					  win_ticks = 0;
					  if ( MessageUpdatePage() == true )
					  {
						 msg_type |= WINDOW_DISTORY; 
						 return ( msg_type ^ WINDOW_TOUCH ); 
					  }
					  
					  osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
                  }
                }
            }else{
                // 无Title且无Body的通知其他通知(闹钟、手机防丢)：退出提醒界面
                if ( finder.cancelFindMe == 1 ) /* FindPhone任务模拟的"Touch"事件,她想提前结束提醒 */
                {
                    finder.cancelFindMe = 0;
                }else{
            
                    if ( notify_id == NOTIFY_ID_FINDME ) /* 手机查找设备中,发送一个确认 */
                    {
                        osal_set_event( taskFinderTaskId, FINDPHONE_TASK_FINDME_RESPONSE_EVT );
                    }
                }

                mq.notify_id = 0;
                msg_type |= WINDOW_DISTORY; // 结束被窗体，从窗体栈恢复上个窗体               
            }
            
        }else{        /** 提醒结束了才触摸按键: 重新点亮屏幕 */
           
            if ( config.lcdEnable == 0 ) /* LCD is off, Power on It */
            {
                /*lcd_display(1);*/
				osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
                mq.notify_id = notify_id;
            }else
			{   
				
			}  

			win_ticks = 0;
        }
		
		
		return ( msg_type ^ WINDOW_TOUCH );
    }
	
	/**
	 * 长按事件
	 */
	if ( msg_type & WINDOW_PRESS )
	{
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
			win_ticks = 0;
			osal_set_event( taskGUITaskId, TASK_GUI_UPDATE_EVT );
		}else{
			
			if ( notify_id == NOFIFY_ID_CALLING )
			{
				if ( ancs_perform_notification_action(false) == false)
				{
					bt_protocol_tx(hangup_cmd, sizeof(hangup_cmd));
				}
			}
			
			mq.notify_id = 0;
			msg_type |= WINDOW_DISTORY; // 结束被窗体，从窗体栈恢复上个窗体			
		}
		
		return ( msg_type ^ WINDOW_PRESS );
	}

    /* 进入消息通知界面(来电,短信,邮件等.) */
    if ( msg_type & WINDOW_NOTIFY )
    {    
		win_ticks = 0, flag = 0;
		
		window[win_current].wproc(win_current, WINDOW_CREATE, 0, 0);
		
		return ( msg_type ^ WINDOW_NOTIFY );
    } 
    
    /**
     * 销毁窗体：擦除自己修改过的区域.
     */
    if ( msg_type & WINDOW_DISTORY )
    {
        MOTOR_ENABLE(0);
      
		window[win_current].arg = 0;
		window[win_current].msg_type = 0;
		window[win_current].param = 0;
		
		// ancs_perform_notification_action( false);
		
		window_pop(&win_current);
		
		return window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0 );
    }
    
    return (0);
}
