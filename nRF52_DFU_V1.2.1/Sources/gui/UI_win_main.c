
#include "string.h"
#include "Window.h"

#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_GUI.h"

#include "LCD_ST7735.h"
#include "UserSetting.h"
#include "UI_Draw.h"

#include "UI_main_icon.h"
#include "ClockTick.h"
#include "Task_Step.h"
#include "Task_Battery.h"
#include "Task_Flash.h"
#include "UI_function_icon.h"
#include "UI_win_function.h"

#define PEDO_0() (pedometer.counter%10)
#define PEDO_1() ((pedometer.counter/10)%10)
#define PEDO_2() ((pedometer.counter/100)%10)
#define PEDO_3() ((pedometer.counter/1000)%10)

#define COLORREF(r,g,b) ((( r & 0xF8)<<8)|(( g & 0xFC)<<3)|(b >> 3))



void UI_Draw_Title(void)
{
	static unsigned char idx = 0;
	unsigned char level;
	
	if ( power.battery_level > 75 )
	{
		level = 4;
	}else if ( power.battery_level > 45 )
	{
		level = 3;
	}else if ( power.battery_level > 15 )
	{
		level = 2;
	}else if ( power.battery_level > 7 ){
		level = 1;
	}else{
		level = 0;
	}

	if ( power.charge_flage == 1 )
	{
		DrawUI_Bitmap_Ex(62,0,19,16,bmp_battery[idx],0x3F<<5);	
		idx++; idx %= 4;
	}else{
						
			switch ( level )
			{
					case 4:
						DrawUI_Bitmap_Ex(62,0,19,16,bmp_battery[3],0x3F<<5);
						break;	
					case 3:
						DrawUI_Bitmap_Ex(62,0,19,16,bmp_battery[2],0x3F<<5);
						break;	
					case 2:
						DrawUI_Bitmap_Ex(62,0,19,16,bmp_battery[1],0x3F<<5);
						break;	
					case 1:
						DrawUI_Bitmap_Ex(62,0,19,16,bmp_battery[1],0x1F<<11);
						break;	
					case 0:
						DrawUI_Bitmap_Ex(62,0,19,16,bmp_battery[0],0x1F<<11);
						break;				
				}
			}					
}

void UI_Draw_Title_Bluetooth(void)
{
	if ( ble.isConnected == 0 )
	{		
		DrawUI_Bitmap_Rever(8,0,12,16,bmp_bluetooth[0],0x1F<<0);
	}else
	{
		DrawUI_Bitmap_Rever(4,0,12,16,bmp_bluetooth[1],0x1F<<0);
	}	
}

void UI_Draw_Picture(int x, int y, int w, int h, const unsigned char *p)
{
	draw_bitmap(x, y, w, h, p);
}

void show_am(int x, int y, int am)
{
	if ( am == 0 )
	{
		UI_Draw_AM( x, y, 0);
		UI_Draw_AM( x+8, y, 1);
	}else{
		UI_Draw_AM( x, y, 2);
		UI_Draw_AM( x+8, y, 3);		
	}
}

void show_week_old( int x, int y, int week)
{
	UI_Draw_Week_old(x, y, week);	
}

int level_convert( int level )
{
	if ( level > 75 )
	{
		return 4;
	}else if ( level > 55 )
	{
		return 3;
	}else if ( level > 40 )
	{
		return 2;
	}else if ( level > 20 )
	{
		return 1;
	}else 
	{
		return 0;
	}
}

#define ALIGN_CENTER 0
#define ALIGN_LEFT   1
#define ALIGN_RIGHT  2

#define FONT_7X9_W 7
#define FONT_7X9_H 9

typedef struct
{
	int w, h;
	unsigned char *p;
}Font;

Font Font7X9 = {
	.w = FONT_7X9_W,
	.h = FONT_7X9_H,
	.p = (unsigned char *)Font7x9_2,
};

Font Font7X9_2 = {
	.w = FONT_7X9_W,
	.h = FONT_7X9_H,
	.p = (unsigned char *)Font_7x9,
};

void ui_show_val( int x, int y, int val, Font * font, int align)
{
	int width = 0, v = val;
	int w, h, idx, sx, ex;
	
	w = font->w; h = font->h;
	
	unsigned char *p;
	
	p = (unsigned char *)font->p;
	
	do{
		width++;
		v /= 10;
	}while(v>0);
	
	if ( align == ALIGN_CENTER )
	{
		sx = x+((80-x) - width*w)/2;
	}else if ( align == ALIGN_LEFT )
	{
		sx = x;
	}else if ( align == ALIGN_RIGHT )
	{
		sx = 80-width*w;
	}else{
		sx = x; // defalut
	}
	
	ex = sx + (width-1) * w;
	do
	{
		idx = val % 10;
		draw_bitmap(ex, y, w, h, p+idx*(w*h*2));
		val /= 10; ex -= w;
	}while( val > 0 );
}

#define COLOR_F 0xFFFF
#define COLOR_B 0x0000

#include "UI_sport_icon.h"
#include "UI_comm_icon.h"


#if 0
void show_main_ui_style_0(void)
{
	UI_Draw_Title_Bluetooth();
	UI_Draw_Title();
	

	int hour = TimerHH();
	if ( config.time_format == 1 )
	{
		if ( hour > 11 )
		{
			hour -= 12;
		}
		
		if ( hour == 0 )
		{ 
			hour = 12; 
		}
	}
	
	// 时间
    DrawUI_Bitmap_Ex(8,32,28,40, Font_28x40[hour/10],0xFFFF);
	DrawUI_Bitmap_Ex(80-8-28,32,28,40, Font_28x40[hour%10],0xFFFF);
	DrawUI_Bitmap_Ex(8,88,28,40, Font_28x40[20+TimerMI()/10],0xFFFF);
	DrawUI_Bitmap_Ex(80-8-28,88,28,40, Font_28x40[20+TimerMI()%10],0xFFFF);
	
	#if 0
    DrawUI_Bitmap_Ex(8,32,28,40, Font_28x40[10+hour/10],0x001F);
	DrawUI_Bitmap_Ex(80-8-28,32,28,40, Font_28x40[10+hour%10],0x001F);
	DrawUI_Bitmap_Ex(8,88,28,40, Font_28x40[10+TimerMI()/10],0x001F);
	DrawUI_Bitmap_Ex(80-8-28,88,28,40, Font_28x40[10+TimerMI()%10],0x001F);
    #endif
	
	// 日期
	DrawUI_Bitmap_Ex( 8,144, 7, 16,Font_7x16[TimerMM()/10], 0XFCE0);
	DrawUI_Bitmap_Ex(15,144, 7, 16,Font_7x16[TimerMM()%10], 0XFCE0);
	DrawUI_Bitmap_Ex(23,144, 7, 16,Font_7x16[12], 0XFCE0);
	DrawUI_Bitmap_Ex(31,144, 7, 16,Font_7x16[TimerDD()/10], 0XFCE0);
	DrawUI_Bitmap_Ex(38,144, 7, 16,Font_7x16[TimerDD()%10], 0XFCE0);	

	// 格式
	if ( config.time_format == 1 )
	{
		if ( TimerHH() > 11 )
		{
			DrawUI_Bitmap_Ex(56, 144, 16, 16, bmp_pm,0X0770);
		}else{
			DrawUI_Bitmap_Ex(56, 144, 16, 16, bmp_am,0X0770);
		}
		
		return;
	}	

	// 星期
    switch ( TimerWK() )
    {	
    case 0:
      DrawUI_Bitmap_Rever(40+10,144,8,16,Font_8x16[12+'S'-'A'],0X0770);
      DrawUI_Bitmap_Rever(48+10,144,8,16,Font_8x16[12+'U'-'A'],0X0770);
      DrawUI_Bitmap_Rever(56+10,144,8,16,Font_8x16[12+'N'-'A'],0X0770);  
      break;
    case 1:
      DrawUI_Bitmap_Rever(40+10,144,8,16,Font_8x16[12+'M'-'A'],0X0770);
      DrawUI_Bitmap_Rever(48+10,144,8,16,Font_8x16[12+'O'-'A'],0X0770);
      DrawUI_Bitmap_Rever(56+10,144,8,16,Font_8x16[12+'N'-'A'],0X0770);  
      break;
    case 2:
      DrawUI_Bitmap_Rever(40+10,144,8,16,Font_8x16[12+'T'-'A'],0X0770);
      DrawUI_Bitmap_Rever(48+10,144,8,16,Font_8x16[12+'U'-'A'],0X0770);
      DrawUI_Bitmap_Rever(56+10,144,8,16,Font_8x16[12+'E'-'A'],0X0770);  
      break;
    case 3:
      DrawUI_Bitmap_Rever(40+10,144,8,16,Font_8x16[12+'W'-'A'],0X0770);
      DrawUI_Bitmap_Rever(48+10,144,8,16,Font_8x16[12+'E'-'A'],0X0770);
      DrawUI_Bitmap_Rever(56+10,144,8,16,Font_8x16[12+'D'-'A'],0X0770); 
      break;
    case 4:
      DrawUI_Bitmap_Rever(40+10,144,8,16,Font_8x16[12+'T'-'A'],0X0770);
      DrawUI_Bitmap_Rever(48+10,144,8,16,Font_8x16[12+'H'-'A'],0X0770);
      DrawUI_Bitmap_Rever(56+10,144,8,16,Font_8x16[12+'U'-'A'],0X0770);  
      break;
    case 5:
      DrawUI_Bitmap_Rever(40+10,144,8,16,Font_8x16[12+'F'-'A'],0X0770);
      DrawUI_Bitmap_Rever(48+10,144,8,16,Font_8x16[12+'R'-'A'],0X0770);
      DrawUI_Bitmap_Rever(56+10,144,8,16,Font_8x16[12+'I'-'A'],0X0770); 
      break;
    case 6:
      DrawUI_Bitmap_Rever(40+10,144,8,16,Font_8x16[12+'S'-'A'],0X0770);
      DrawUI_Bitmap_Rever(48+10,144,8,16,Font_8x16[12+'A'-'A'],0X0770);
      DrawUI_Bitmap_Rever(56+10,144,8,16,Font_8x16[12+'T'-'A'],0X0770);  
      break;
    }	
}
#else

//const unsigned char bmp_ble_[] = {0x04,0x01,0x88,0x00,0x50,0x00,0xFF,0x07,0x21,0x04,0x22,0x02,0x54,0x01,0x88,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};/*"F:\BandAll\HB021\彩屏niyajia\ble.BMP",0*/
//const unsigned char bmp_battery[][38] = /** 19x16 */
//{
//{0x00,0x00,0xF0,0x0F,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x70,0x0E,0xC0,0x03,0x00,0x00,0x00,0x00},/*"F:\Band\HB021\尼加亚彩屏_80x160\尼加亚彩屏_80x160\切图\主界面\battery\battery_level_0.BMP",0*/
//{0x00,0x00,0xF0,0x0F,0x10,0x08,0xD0,0x0B,0xD0,0x0B,0xD0,0x0B,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x70,0x0E,0xC0,0x03,0x00,0x00,0x00,0x00},/*"F:\Band\HB021\尼加亚彩屏_80x160\尼加亚彩屏_80x160\切图\主界面\battery\battery_level_1.BMP",0*/
//{0x00,0x00,0xF0,0x0F,0x10,0x08,0xD0,0x0B,0xD0,0x0B,0xD0,0x0B,0x10,0x08,0xD0,0x0B,0xD0,0x0B,0xD0,0x0B,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x70,0x0E,0xC0,0x03,0x00,0x00,0x00,0x00},/*"F:\Band\HB021\尼加亚彩屏_80x160\尼加亚彩屏_80x160\切图\主界面\battery\battery_level_2.BMP",0*/
//{0x00,0x00,0xF0,0x0F,0x10,0x08,0xD0,0x0B,0xD0,0x0B,0xD0,0x0B,0x10,0x08,0xD0,0x0B,0xD0,0x0B,0xD0,0x0B,0x10,0x08,0xD0,0x0B,0xD0,0x0B,0xD0,0x0B,0x10,0x08,0x70,0x0E,0xC0,0x03,0x00,0x00,0x00,0x00},/*"F:\Band\HB021\尼加亚彩屏_80x160\尼加亚彩屏_80x160\切图\主界面\battery\battery_level_3.BMP",0*/
//};

const unsigned char bmp_ble_[] = 

{0x00,0x00,0x7C,0x00,0x82,0x00,0x92,0x00,0x92,0x00,0x92,0x00,0x10,0x00,0x10,0x00,0x92,0x00,0x92,0x00,0x92,0x00,0x82,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00};/*"E:\BandAll\HB045_GIT\UI\HB045图标调整_6.19\HB045图标调整_6.19\ble.BMP",0*/ 




const unsigned char Font_9x16[][18] = 
{
{0x00,0x00,0xF8,0x3F,0xFC,0x7F,0x0C,0x60,0x0C,0x60,0x0C,0x60,0xFC,0x7F,0xF8,0x3F,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\0.bmp",0*/
{0x00,0x00,0x00,0x00,0x30,0x00,0x18,0x00,0xFC,0x7F,0xFC,0x7F,0x00,0x00,0x00,0x00,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\1.bmp",0*/
{0x00,0x00,0x78,0x70,0x7C,0x7C,0x0C,0x6E,0x8C,0x67,0xCC,0x61,0xFC,0x60,0x78,0x60,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\2.bmp",0*/
{0x00,0x00,0x38,0x38,0x3C,0x78,0x0C,0x60,0x8C,0x61,0x8C,0x61,0xFC,0x7F,0x78,0x3E,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\3.bmp",0*/
{0x00,0x00,0x00,0x0E,0x80,0x0F,0xC0,0x0F,0xF0,0x0C,0x3C,0x0C,0x1C,0x7F,0x04,0x7F,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\4.bmp",0*/
{0x00,0x00,0xFC,0x39,0xFC,0x79,0x8C,0x61,0x8C,0x61,0x8C,0x61,0x8C,0x7F,0x0C,0x3F,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\5.bmp",0*/
{0x00,0x00,0xF8,0x3F,0xFC,0x7F,0x8C,0x61,0x8C,0x61,0x8C,0x61,0xB8,0x7F,0x30,0x3F,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\6.bmp",0*/
{0x00,0x00,0x00,0x00,0x0C,0x40,0x0C,0x78,0x0C,0x3F,0xEC,0x07,0xFC,0x00,0x1C,0x00,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\7.bmp",0*/
{0x00,0x00,0x78,0x3E,0xFC,0x7F,0x8C,0x61,0x8C,0x61,0x8C,0x61,0xFC,0x7F,0x78,0x3E,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\8.bmp",0*/
{0x00,0x00,0xF8,0x19,0xFC,0x3B,0x0C,0x63,0x0C,0x63,0x0C,0x63,0xFC,0x7F,0xF8,0x3F,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\大数字\9.bmp",0*/
};

const unsigned char gImage_pedomete_icon_s[162] = { /* 0X00,0X10,0X09,0X00,0X09,0X00,0X01,0X1B, */
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X60,0X00,0X61,0X06,0X61,0X07,
0XA0,0X02,0X00,0X00,0X00,0X00,0X60,0X01,0XA1,0X06,0XC1,0X07,0X80,0X03,0XE1,0X07,
0XE1,0X07,0X41,0X07,0X00,0X00,0X00,0X00,0XE1,0X05,0XE1,0X07,0XE1,0X07,0XC0,0X03,
0XE1,0X07,0XE1,0X07,0XA1,0X07,0X00,0X00,0X00,0X00,0X21,0X07,0XE1,0X07,0XE1,0X07,
0XC0,0X03,0XE1,0X07,0XE1,0X07,0X61,0X05,0X00,0X00,0X00,0X00,0X21,0X07,0XE1,0X07,
0XE1,0X06,0X00,0X01,0X00,0X04,0X00,0X04,0X80,0X01,0X00,0X00,0X00,0X00,0XE0,0X04,
0X60,0X04,0XE0,0X01,0XC0,0X00,0XA1,0X07,0X01,0X07,0X20,0X00,0X00,0X00,0X00,0X00,
0X21,0X06,0X01,0X07,0XC0,0X00,0X00,0X00,0XA0,0X00,0X60,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,};

const unsigned char gImage_distance_s[192] = {0x00,0x07,0x05,0x07,0x02,0x02,0x7E,0x40,0xE0,0xA0,0xE0,0x00};/*"未命名文件",0*/


const unsigned char Font5x8[][5] = 
{
{0x7E,0x81,0x81,0x81,0x7E},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\0.bmp",0*/
{0x00,0x02,0xFF,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\1.bmp",0*/
{0x71,0x89,0x89,0x89,0x86},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\2.bmp",0*/
{0x89,0x89,0x89,0x89,0x76},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\3.bmp",0*/
{0x07,0x08,0x08,0x08,0xFF},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\4.bmp",0*/
{0x86,0x89,0x89,0x89,0x71},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\5.bmp",0*/
{0x7E,0x89,0x89,0x89,0x71},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\6.bmp",0*/
{0x01,0x01,0x01,0x01,0xFE},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\7.bmp",0*/
{0x76,0x89,0x89,0x89,0x76},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\8.bmp",0*/
{0x86,0x89,0x89,0x89,0x7E},/*"F:\BandAll\HB021\彩屏niyajia\界面2\与界面3数字相同\9.bmp",0*/
};

const unsigned char gImage_s_hr[192] = { /* 0X00,0X10,0X0C,0X00,0X08,0X00,0X01,0X1B, */
0X00,0X00,0X00,0X28,0X00,0XD0,0X00,0XF8,0X00,0XB8,0X00,0X28,0X00,0XB8,0X00,0XF8,
0X00,0XD0,0X00,0X28,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XC8,0X00,0XF8,0X00,0XF8,
0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XC8,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,
0X00,0XF8,0X00,0XF8,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XE0,0X00,0XF8,0X00,0XF8,
0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XE0,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X78,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,
0X00,0XF8,0X00,0X78,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0XC0,0X00,0XF8,
0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XC0,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X00,0X08,0X00,0XC0,0X00,0XF8,0X00,0XF8,0X00,0XF8,0X00,0XC0,
0X00,0X08,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X80,0X00,0XE8,0X00,0X80,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};

const unsigned char gImage_s_calurie[160] = { /* 0X00,0X10,0X08,0X00,0X0A,0X00,0X01,0X1B, */
0X00,0X00,0X00,0X00,0X00,0X00,0XE0,0X91,0X80,0X20,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X40,0X61,0X20,0XF3,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
0X00,0X00,0X00,0X00,0X40,0XAA,0X60,0XFB,0X40,0X18,0X00,0X00,0X40,0X18,0X20,0X10,
0X00,0X00,0XE0,0X48,0X00,0X9A,0X60,0XFB,0X00,0X92,0X60,0X18,0X00,0XEB,0X20,0X08,
0X00,0X00,0XE0,0XDA,0XE0,0X91,0X60,0XFB,0X60,0XFB,0X80,0XC2,0X60,0XFB,0X60,0X20,
0XA0,0X30,0X60,0XFB,0XC0,0XD2,0X60,0XFB,0XE0,0XDA,0X60,0XFB,0X60,0XFB,0XA0,0X81,
0XA0,0XCA,0X60,0XFB,0X40,0XF3,0X60,0XFB,0X40,0X59,0X60,0XFB,0X60,0XFB,0X00,0XE3,
0X60,0XFB,0X60,0XFB,0X40,0XAA,0X00,0X51,0XE0,0X40,0X60,0XBA,0X60,0XFB,0X40,0XFB,
0X60,0XB2,0X60,0XFB,0X40,0X61,0X00,0X00,0X20,0X08,0XA0,0X30,0X60,0XFB,0X00,0X9A,
0X20,0X08,0XA0,0X81,0XC0,0X81,0X20,0X08,0X00,0X00,0XA0,0X28,0X80,0X71,0X00,0X00,
};

const unsigned char battery_level_style_1[][40] = 
{
{0x00,0x00,0x00,0x00,0x7E,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\电池\battery_0.BMP",0*/
{0x00,0x00,0x00,0x00,0x7E,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\电池\battery_1.BMP",0*/
{0x00,0x00,0x00,0x00,0x7E,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\电池\battery_2.BMP",0*/
{0x00,0x00,0x00,0x00,0x7E,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x81,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\电池\battery_3.BMP",0*/
{0x00,0x00,0x00,0x00,0x7E,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0xBD,0x00,0xBD,0x00,0x81,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00},/*"F:\BandAll\HB021\彩屏niyajia\电池\battery_4.BMP",0*/
};

void draw_battery( int x, int y, int color )
{
	static unsigned char idx = 0;
	unsigned char level;
	
	if ( power.battery_level > 75 )
	{
		level = 4;
	}else if ( power.battery_level > 45 )
	{
		level = 3;
	}else if ( power.battery_level > 15 )
	{
		level = 2;
	}else if ( power.battery_level > 7 ){
		level = 1;
	}else{
		level = 0;
	}

	if ( power.charge_flage == 1 )
	{
		DrawUI_Bitmap_Ex(x,y,20,16,battery_level_style_1[idx],color);
		idx++; idx %= 5;
	}else{

		switch ( level )
		{
		case 4:
			DrawUI_Bitmap_Ex(x,y,20,16,battery_level_style_1[4],color);
			break;	
		case 3:
			DrawUI_Bitmap_Ex(x,y,20,16,battery_level_style_1[3],color);
			break;	
		case 2:
			DrawUI_Bitmap_Ex(x,y,20,16,battery_level_style_1[2],color);
			break;	
		case 1:
			DrawUI_Bitmap_Ex(x,y,20,16,battery_level_style_1[1],color);
			break;	
		case 0:
			DrawUI_Bitmap_Ex(x,y,20,16,battery_level_style_1[0],0x1F<<11);
			break;				
		}
	}	
}

const unsigned char bmp_month_7x12[][3][12] = 
{
{


{0x00,0x06,0x04,0x04,0x04,0x04,0xFC,0x03,0x04,0x00,0x04,0x00},/*"J",0*/

{0x00,0x02,0xE0,0x03,0x9C,0x00,0xF0,0x00,0x80,0x03,0x00,0x02},/*"A",1*/

{0x04,0x02,0xFC,0x03,0x30,0x02,0xC4,0x00,0xFC,0x03,0x04,0x00},/*"N",2*/



},
{


{0x04,0x02,0xFC,0x03,0x24,0x02,0x74,0x00,0x0C,0x00,0x00,0x00},/*"F",0*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x74,0x02,0x0C,0x03,0x00,0x00},/*"E",1*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x24,0x02,0xD8,0x01,0x00,0x00},/*"B",2*/


},

{


{0xFC,0x03,0x3C,0x00,0xC0,0x03,0x3C,0x00,0xFC,0x03,0x00,0x00},/*"M",0*/

{0x00,0x02,0xE0,0x03,0x9C,0x00,0xF0,0x00,0x80,0x03,0x00,0x02},/*"A",1*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x64,0x00,0x98,0x03,0x00,0x02},/*"R",2*/



},
{


{0x00,0x02,0xE0,0x03,0x9C,0x00,0xF0,0x00,0x80,0x03,0x00,0x02},/*"A",0*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x24,0x00,0x18,0x00,0x00,0x00},/*"P",1*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x64,0x00,0x98,0x03,0x00,0x02},/*"R",2*/





},
{

{0xFC,0x03,0x3C,0x00,0xC0,0x03,0x3C,0x00,0xFC,0x03,0x00,0x00},/*"M",0*/

{0x00,0x02,0xE0,0x03,0x9C,0x00,0xF0,0x00,0x80,0x03,0x00,0x02},/*"A",1*/

{0x04,0x00,0x1C,0x02,0xE0,0x03,0x1C,0x02,0x04,0x00,0x00,0x00},/*"Y",2*/


},
{





{0x00,0x06,0x04,0x04,0x04,0x04,0xFC,0x03,0x04,0x00,0x04,0x00},/*"J",0*/

{0x04,0x00,0xFC,0x01,0x00,0x02,0x00,0x02,0xFC,0x01,0x04,0x00},/*"U",1*/

{0x04,0x02,0xFC,0x03,0x30,0x02,0xC4,0x00,0xFC,0x03,0x04,0x00},/*"N",2*/



},
{




{0x00,0x06,0x04,0x04,0x04,0x04,0xFC,0x03,0x04,0x00,0x04,0x00},/*"J",0*/

{0x04,0x00,0xFC,0x01,0x00,0x02,0x00,0x02,0xFC,0x01,0x04,0x00},/*"U",1*/

{0x04,0x02,0xFC,0x03,0x04,0x02,0x00,0x02,0x00,0x02,0x00,0x03},/*"L",2*/



},
{




{0x00,0x02,0xE0,0x03,0x9C,0x00,0xF0,0x00,0x80,0x03,0x00,0x02},/*"A",0*/

{0x04,0x00,0xFC,0x01,0x00,0x02,0x00,0x02,0xFC,0x01,0x04,0x00},/*"U",1*/

{0xF0,0x00,0x08,0x01,0x04,0x02,0x44,0x02,0xCC,0x01,0x40,0x00},/*"G",2*/



},
{



{0x18,0x03,0x24,0x02,0x24,0x02,0x44,0x02,0x8C,0x01,0x00,0x00},/*"S",0*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x74,0x02,0x0C,0x03,0x00,0x00},/*"E",1*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x24,0x00,0x18,0x00,0x00,0x00},/*"P",2*/



},
{

{0xF8,0x01,0x04,0x02,0x04,0x02,0x04,0x02,0xF8,0x01,0x00,0x00},/*"O",0*/

{0xF8,0x01,0x04,0x02,0x04,0x02,0x04,0x02,0x0C,0x01,0x00,0x00},/*"C",1*/

{0x0C,0x00,0x04,0x02,0xFC,0x03,0x04,0x02,0x0C,0x00,0x00,0x00},/*"T",2*/


},
{


{0x04,0x02,0xFC,0x03,0x30,0x02,0xC4,0x00,0xFC,0x03,0x04,0x00},/*"N",0*/

{0xF8,0x01,0x04,0x02,0x04,0x02,0x04,0x02,0xF8,0x01,0x00,0x00},/*"O",1*/

{0x04,0x00,0x7C,0x00,0x80,0x03,0xE0,0x00,0x1C,0x00,0x04,0x00},/*"V",2*/


},
{



{0x04,0x02,0xFC,0x03,0x04,0x02,0x04,0x02,0xF8,0x01,0x00,0x00},/*"D",0*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x74,0x02,0x0C,0x03,0x00,0x00},/*"E",1*/

{0xF8,0x01,0x04,0x02,0x04,0x02,0x04,0x02,0x0C,0x01,0x00,0x00},/*"C",2*/


},	
};

const unsigned char Font_Ascii_N_8x12[][16] = 
{

{0x00,0x00,0xF0,0x00,0x0C,0x03,0x04,0x02,0x04,0x02,0x0C,0x03,0xF0,0x00,0x00,0x00},/*"0",0*/

{0x00,0x00,0x08,0x02,0x08,0x02,0xFC,0x03,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00},/*"1",1*/

{0x00,0x00,0x18,0x03,0x04,0x03,0x84,0x02,0x44,0x02,0x24,0x02,0x18,0x03,0x00,0x00},/*"2",2*/

{0x00,0x00,0x08,0x01,0x04,0x02,0x24,0x02,0x24,0x02,0x24,0x02,0xD8,0x01,0x00,0x00},/*"3",3*/

{0x00,0x00,0x60,0x00,0x90,0x00,0x88,0x00,0x84,0x02,0xFC,0x03,0x00,0x00,0x00,0x00},/*"4",4*/

{0x00,0x00,0x3C,0x01,0x24,0x02,0x24,0x02,0x24,0x02,0x24,0x02,0xC4,0x01,0x00,0x00},/*"5",5*/

{0x00,0x00,0xF0,0x01,0x48,0x03,0x24,0x02,0x24,0x02,0x2C,0x02,0xC0,0x01,0x00,0x00},/*"6",6*/

{0x00,0x00,0x0C,0x00,0x04,0x00,0xC4,0x03,0x34,0x00,0x0C,0x00,0x04,0x00,0x00,0x00},/*"7",7*/

{0x00,0x00,0x98,0x01,0x64,0x02,0x24,0x02,0x44,0x02,0x64,0x02,0x98,0x01,0x00,0x00},/*"8",8*/

{0x00,0x00,0x38,0x00,0x48,0x03,0x44,0x02,0x44,0x02,0x28,0x03,0xF0,0x00,0x00,0x00},/*"9",9*/

{0x00,0x00,0x00,0x04,0x00,0x03,0x80,0x00,0x60,0x00,0x18,0x00,0x0C,0x00,0x02,0x00},/*"/",10*/	
};


const unsigned char bmp_week_2[][3][12] = 
{
{

{0x18,0x03,0x24,0x02,0x24,0x02,0x44,0x02,0x8C,0x01,0x00,0x00},/*"S",0*/

{0x04,0x00,0xFC,0x01,0x00,0x02,0x00,0x02,0xFC,0x01,0x04,0x00},/*"U",1*/

{0x04,0x02,0xFC,0x03,0x30,0x02,0xC4,0x00,0xFC,0x03,0x04,0x00},/*"N",2*/

},
{


{0xFC,0x03,0x3C,0x00,0xC0,0x03,0x3C,0x00,0xFC,0x03,0x00,0x00},/*"M",0*/

{0xF8,0x01,0x04,0x02,0x04,0x02,0x04,0x02,0xF8,0x01,0x00,0x00},/*"O",1*/

{0x04,0x02,0xFC,0x03,0x30,0x02,0xC4,0x00,0xFC,0x03,0x04,0x00},/*"N",2*/

},
{

{0x0C,0x00,0x04,0x02,0xFC,0x03,0x04,0x02,0x0C,0x00,0x00,0x00},/*"T",0*/

{0x04,0x00,0xFC,0x01,0x00,0x02,0x00,0x02,0xFC,0x01,0x04,0x00},/*"U",1*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x74,0x02,0x0C,0x03,0x00,0x00},/*"E",2*/

},
{

{0x1C,0x00,0xE0,0x03,0x3C,0x00,0xE0,0x03,0x1C,0x00,0x00,0x00},/*"W",0*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x74,0x02,0x0C,0x03,0x00,0x00},/*"E",1*/

{0x04,0x02,0xFC,0x03,0x04,0x02,0x04,0x02,0xF8,0x01,0x00,0x00},/*"D",2*/

},
{

{0x0C,0x00,0x04,0x02,0xFC,0x03,0x04,0x02,0x0C,0x00,0x00,0x00},/*"T",0*/

{0x04,0x02,0xFC,0x03,0x20,0x00,0x20,0x00,0xFC,0x03,0x04,0x02},/*"H",1*/

{0x04,0x00,0xFC,0x01,0x00,0x02,0x00,0x02,0xFC,0x01,0x04,0x00},/*"U",2*/

},
{

{0x04,0x02,0xFC,0x03,0x24,0x02,0x74,0x00,0x0C,0x00,0x00,0x00},/*"F",0*/

{0x04,0x02,0xFC,0x03,0x24,0x02,0x64,0x00,0x98,0x03,0x00,0x02},/*"R",1*/

{0x04,0x02,0x04,0x02,0xFC,0x03,0x04,0x02,0x04,0x02,0x00,0x00},/*"I",2*/

},
{


{0x18,0x03,0x24,0x02,0x24,0x02,0x44,0x02,0x8C,0x01,0x00,0x00},/*"S",0*/

{0x00,0x02,0xE0,0x03,0x9C,0x00,0xF0,0x00,0x80,0x03,0x00,0x02},/*"A",1*/

{0x0C,0x00,0x04,0x02,0xFC,0x03,0x04,0x02,0x0C,0x00,0x00,0x00},/*"T",2*/

},	
};


void DrawDateWeek__(int x, int y)
{
	// date
	int h,i;
	
	h = TimerDD();
	
	DrawUI_Bitmap_Ex( x+0*8+17,y,8,16,Font_Ascii_N_8x12[10],COLORREF(150,150,150));
	DrawUI_Bitmap_Ex( x+1*8+17,y,8,16,Font_Ascii_N_8x12[h/10],COLORREF(150,150,150));
	DrawUI_Bitmap_Ex( x+2*8+17,y,8,16,Font_Ascii_N_8x12[h%10],COLORREF(150,150,150));
	DrawUI_Bitmap_Ex( x+3*8+17,y,8,16,Font_Ascii_N_8x12[10],COLORREF(150,150,150));

	// month
	
	h = TimerMM();

	i = 0;
	for ( int j = 0; j < 3; j++,i++ )
	{
		DrawUI_Bitmap_Ex( x+i*6,y,6,16,bmp_month_7x12[h][j],COLORREF(150,150,150));
	}
	
	
	
	// week
	h = TimerWK();

	i = 9;
	for ( int j = 0; j < 3; j++,i++ )
	{
		DrawUI_Bitmap_Ex( x+i*6-3, y, 6, 16, bmp_week_2[h][j],COLORREF(150,150,150));
	}
}

#include "jpeg_decode.h"
#define PI 3.14159265       
#include "math.h"

void show_main_ui_style_0(void)
{
	int val, minute, h;
	
	// 绘制背景
	DrawUI_JPEG(0,0,jpeg_ui_bg_1);
	
	// 绘制蓝牙图标和电池图标
	if ( ble.isConnected == 1 )
	{
		DrawUI_Bitmap_Ex(0,0,16,16,bmp_ble_,COLORREF(160,160,160));
	}else{
		DrawUI_Bitmap_Ex(0,0,16,16,bmp_ble_,COLORREF(60,60,60));
	}
	
	draw_battery(60,1,COLORREF(150,150,150));	
	
	// 显示分钟，分两种情况：时针在上面时，分钟在下面显示
	//                       时针在下面时，分钟在上面显示
	h = TimerHH();
	
	if ( h >= 3 && h <= 8 ) 
	{
		minute = TimerMI();
		DrawUI_Bitmap_Ex((80-16)/2-2,(160-16)/2-16,9,16,Font_9x16[minute/10],COLORREF(200,200,200));
		DrawUI_Bitmap_Ex((80-16)/2+9-2,(160-16)/2-16,9,16,Font_9x16[minute%10],COLORREF(200,200,200));
	}else{
		minute = TimerMI();
		DrawUI_Bitmap_Ex((80-16)/2-2,(160-16)/2-16+22,9,16,Font_9x16[minute/10],COLORREF(200,200,200));
		DrawUI_Bitmap_Ex((80-16)/2+9-2,(160-16)/2-16+22,9,16,Font_9x16[minute%10],COLORREF(200,200,200));		
	}
	
	// 绘制步数图标和步数值
	draw_bitmap(10,33,9,9,gImage_pedomete_icon_s);
	
	val = pedometer.counter % 100000;
	DrawUI_Bitmap_Ex( 6+4,23,5,8,Font5x8[val/10000],COLORREF(150,150,150));
	val %= 10000;
	DrawUI_Bitmap_Ex(11+4,23,5,8,Font5x8[val/1000],COLORREF(150,150,150));
	val %= 1000;
	DrawUI_Bitmap_Ex(17+4,23,5,8,Font5x8[val/100],COLORREF(150,150,150));
	val %= 100;
	DrawUI_Bitmap_Ex(23+4,23,5,8,Font5x8[val/10],COLORREF(150,150,150));
	val %= 10;
	DrawUI_Bitmap_Ex(29+4,23,5,8,Font5x8[val],COLORREF(150,150,150));
	
	// 绘制距离图标和距离值
	DrawUI_Bitmap_Ex(58,34,12,8,gImage_distance_s,0X8BE0);
	
	val = pedometer.distance % 10000;
	DrawUI_Bitmap_Ex( 6+38,23,5,8,Font5x8[val/1000],COLORREF(150,150,150));
	val %= 1000;
	DrawUI_Bitmap_Ex(11+38,23,5,8,Font5x8[val/100],COLORREF(150,150,150));
	val %= 100;
	DrawUI_Bitmap_Ex(17+38,23,5,8,Font5x8[val/10],COLORREF(150,150,150));
	val %= 10;	
	DrawUI_Bitmap_Ex(23+38,23,5,8,Font5x8[val],COLORREF(150,150,150));
	
	// 绘制心率图标和心率值
	draw_bitmap(10,112,12,8,gImage_s_hr);
	DrawUI_Bitmap_Ex( 6+12,122,5,8,Font5x8[hr.hr/100],COLORREF(150,150,150));
	DrawUI_Bitmap_Ex(11+12,122,5,8,Font5x8[(hr.hr/10)%10],COLORREF(150,150,150));
	DrawUI_Bitmap_Ex(17+12,122,5,8,Font5x8[hr.hr%10],COLORREF(150,150,150));
	
	// 绘制卡路里图标和卡路里值
	draw_bitmap(60,110,8,10,gImage_s_calurie);	
	val = pedometer.calorie % 10000;	
	DrawUI_Bitmap_Ex( 6+36,122,5,8,Font5x8[val/1000],COLORREF(150,150,150));
	val %= 1000;	
	DrawUI_Bitmap_Ex(11+36,122,5,8,Font5x8[val/100],COLORREF(150,150,150));
	val %= 100;	
	DrawUI_Bitmap_Ex(17+36,122,5,8,Font5x8[val/10],COLORREF(150,150,150));
	val %= 10;	
	DrawUI_Bitmap_Ex(23+36,122,5,8,Font5x8[val],COLORREF(150,150,150));	
	

	DrawDateWeek__(6,144);
 
	
	// 绘制分针和时针
	float h_hour,h_minute,h_second;
	
	int x_hour, y_hour;
//    int x_minute, y_minute/*, x_second, y_second*/;
	int hour/*, minute*/, second;
	
	hour = TimerHH();
	minute = TimerMI();
	second = TimerSE();

	h_second = (float)(second * 2 *PI /60.0f);
	h_minute = (float)(minute * 2 * PI/60.0f+h_second/60.0f);
	h_hour = (float)(hour * 2 * PI/12.0f+h_minute/12.0f);
	
	/* x_second = (int)(32*sin(h_second));
	y_second = (int)(32*cos(h_second)); */
	
//	x_minute = (int)(26*sin(h_minute));
//	y_minute = (int)(26*cos(h_minute));
	
	x_hour = (int)(20*sin(h_hour));
	y_hour = (int)(20*cos(h_hour));		
		
	// 绘制时针、分针和秒针
	#if 0
	draw_line( 40+x_hour,   69-y_hour,  40-x_hour/4 , 69+y_hour/4,  0xFFFF); 
	draw_line( 40+x_minute, 69-y_minute,40-x_minute/4,69+y_minute/4,0xFFFF);	
	draw_line( 40+x_second, 69-y_second,40-x_second/4,69+y_second/4,0xFFFF);	
	#else
	draw_line( 40+x_hour,   75-y_hour,  40, 75, COLORREF(200,200,200)); 	
	#endif	
}
#endif

void show_main_ui_style_1( int flag )
{	
	UI_Draw_Title_Bluetooth();
	UI_Draw_Title();
	
	
	int hour = TimerHH();
	
	if ( config.time_format == 1 )
	{
		if ( hour > 11 )
		{
			hour -= 12;
		}
		
		if ( hour == 0 )
		{ 
			hour = 12; 
		}
	}
	
	DrawUI_Bitmap_Ex( 8,24,30,40, Font_30x40[hour/10],colorTable[hour/10]); // hour
	DrawUI_Bitmap_Ex(40,24,30,40,  Font_30x40[hour%10],colorTable[hour%10]);	
	
	DrawUI_Bitmap_Ex( 8,72,30,40,  Font_30x40[TimerMI()/10],colorTable[TimerMI()/10]);// minute
	DrawUI_Bitmap_Ex(40,72,30,40,  Font_30x40[TimerMI()%10],colorTable[TimerMI()%10]);
			
	if ( config.time_format == 1 )      // AM/PM
	{
		if ( TimerHH() > 11 )
		{
			DrawUI_Bitmap_Ex(40, 1, 19, 16, bmp_pm_style_1,0xFFFF);
		}else{
			DrawUI_Bitmap_Ex(40, 1, 19, 16, bmp_am_style_1,0xFFFF);
		}
	}
	
	// 心率
	// 双色
	DrawUI_Bitmap_Ex(4,127,32,32,bmp_hr_1, 0X1F<<11); 
	DrawUI_Bitmap_Ex(4,127,32,32,bmp_hr_2, 0XFFFF);
	
	DrawUI_Bitmap_Ex(44,144,34,16,bmp_bpm,0xFFFF); // bpm
	DrawUI_Bitmap_Ex( 44,128,8,16, Font_8x16[(hr.hr/100)%10],0xFFFF);
	DrawUI_Bitmap_Ex( 52,128,8,16, Font_8x16[(hr.hr/10)%10],0xFFFF);
	DrawUI_Bitmap_Ex( 60,128,8,16, Font_8x16[hr.hr%10],0xFFFF);	
}

#define PI 3.14159265       
#include "math.h"

void UI_Draw_Title_Bluetooth_1(void)
{
	if ( ble.isConnected == 0 )
	{		
		DrawUI_Bitmap_Rever((80-12)/2-2,120,12,16,bmp_bluetooth[0],0x1F<<0);
	}else
	{
		DrawUI_Bitmap_Rever((80-12)/2-1,120,12,16,bmp_bluetooth[1],0x1F<<0);
	}	
}

void UI_Draw_Title_1(void)
{
	static unsigned char idx = 0;
	unsigned char level;
	
	if ( power.battery_level > 75 )
	{
		level = 4;
	}else if ( power.battery_level > 45 )
	{
		level = 3;
	}else if ( power.battery_level > 15 )
	{
		level = 2;
	}else if ( power.battery_level > 7 ){
		level = 1;
	}else{
		level = 0;
	}

	if ( power.charge_flage == 1 )
	{
		DrawUI_Bitmap_Ex((80-19)/2,24,19,16,bmp_battery[idx],0x3F<<5);	
		idx++; idx %= 4;
	}else{
		switch ( level )
		{
		case 4:
			DrawUI_Bitmap_Ex((80-19)/2,24,19,16,bmp_battery[3],0x3F<<5);
			break;
		case 3:
			DrawUI_Bitmap_Ex((80-19)/2,24,19,16,bmp_battery[2],0x3F<<5);
			break;
		case 2:
			DrawUI_Bitmap_Ex((80-19)/2,24,19,16,bmp_battery[1],0x3F<<5);
			break;
		case 1:
			DrawUI_Bitmap_Ex((80-19)/2,24,19,16,bmp_battery[1],0x1F<<11);
			break;
		case 0:
			DrawUI_Bitmap_Ex((80-19)/2,24,19,16,bmp_battery[0],0x1F<<11);
			break;
		}
	}	
}

unsigned char fristBootFlag = 1;
unsigned char showLogoFlag = 1;
	
void show_ui_version(void);
void show_ui_logo(void)
{	
}

#define PI 3.14159265       
#include "math.h"

void DrawBackGroud(void)
{
    memset(frame_buffer,0,sizeof(frame_buffer));
}

#include "Task_UItraviolet.h"

void UI_Draw_Title_Bluetooth_(int x, int y)
{
	if ( ble.isConnected == 0 )
	{		
		DrawUI_Bitmap_Ex(x+4,y,18,16,bmp_bluetooth_[0],0xFFFF<<0);
	}else
	{
		DrawUI_Bitmap_Ex(x+8,y,10,16,bmp_bluetooth_[1],0xFFFF<<0);
	}	
}

// #define COLORREF(a,b,c) ((a>>3)<<11)|((b>>2)<<5)|(c>>3))

const unsigned char *jpeg_ui_level[5] = 
{
jpeg_ui_cycle_0,    
jpeg_ui_cycle_1,   
jpeg_ui_cycle_2,    
jpeg_ui_cycle_3,    
jpeg_ui_cycle_4,   
};

int uit_get_level( int uit_level )
{
    if ( uit_level < 3 )
    {
        return 0;
    }else if ( uit_level < 5 )
    {
        return 1;
    }else if ( uit_level < 7 )
    {
        return 2;
    }else if ( uit_level < 10 )
    {
        return 3;
    }else{
        return 4;
    }
}

const unsigned char bmp_ble_16x10[] = {0xFC,0x00,0xFE,0x01,0x87,0x03,0x03,0x03,0x33,0x03,0x33,0x03,0x33,0x03,0x30,0x00,0x30,0x00,0x33,0x03,0x33,0x03,0x33,0x03,0x03,0x03,0x87,0x03,0xFE,0x01,0xFC,0x00};/*"E:\BandAll\HB045_GIT\UI\HB045图标调整_6.19\HB045图标调整_6.19\bluetooth.bmp",0*/ 

void UI_Draw_Title_Bluetooth_2(void)
{
	if ( ble.isConnected == 0 )
	{		
		DrawUI_Bitmap_Ex(42-2,3,16,16,bmp_ble_16x10,0xFFFF);
	}else
	{
		DrawUI_Bitmap_Ex(42-2,3,16,16,bmp_ble_16x10,0x1F<<0);
	}	
}

void show_uit_win( void)
{
    #if 0
    UIT_adc %= 10000;
    
    DrawUI_Bitmap_Ex( 16+0,48,8,16, Font_8x16[UIT_adc/1000],0xFFFF);
    UIT_adc %= 1000;
    DrawUI_Bitmap_Ex( 16+8,48,8,16, Font_8x16[UIT_adc/100],0xFFFF);
    UIT_adc %= 100;
    DrawUI_Bitmap_Ex(16+16,48,8,16, Font_8x16[UIT_adc/10],0xFFFF); 
    UIT_adc %= 10;
    DrawUI_Bitmap_Ex(16+24,48,8,16, Font_8x16[UIT_adc],0xFFFF);    
    
    int vol = (int)(UIT_vol*1000);

    vol %= 10000;
    
    DrawUI_Bitmap_Ex(16+ 0,64,8,16, Font_8x16[vol/1000],0xFFFF);
    vol %= 1000;
    DrawUI_Bitmap_Ex(16+ 8,64,8,16, Font_8x16[vol/100],0xFFFF);
    vol %= 100;
    DrawUI_Bitmap_Ex(16+16,64,8,16, Font_8x16[vol/10],0xFFFF); 
    vol %= 10;
    DrawUI_Bitmap_Ex(16+24,64,8,16, Font_8x16[vol],0xFFFF);  

    DrawUI_Bitmap_Ex(16+32,64,8,16, Font_8x16_O[0],0xFFFF);  
    DrawUI_Bitmap_Ex(16+40,64,8,16, Font_8x16_O[1],0xFFFF);  
    
    int tmp = (int)(UIT_cm2[0]*1000);
    
    tmp %= 10000;
    
    DrawUI_Bitmap_Ex(16+ 0,80,8,16, Font_8x16[tmp/1000],0xFFFF);
    tmp %= 1000;
    DrawUI_Bitmap_Ex(16+ 8,80,8,16, Font_8x16_O[2],0xFFFF);
    DrawUI_Bitmap_Ex(16+16,80,8,16, Font_8x16[tmp/100],0xFFFF);
    tmp %= 100;
    DrawUI_Bitmap_Ex(16+24,80,8,16, Font_8x16[tmp/10],0xFFFF); 
    tmp %= 10;
    DrawUI_Bitmap_Ex(16+32,80,8,16, Font_8x16[tmp],0xFFFF);
    
    tmp = (int)(UIT_i[0]*1000);
    
    tmp %= 10000;

    DrawUI_Bitmap_Ex(16+ 0,96,8,16, Font_8x16[tmp/1000],0xFFFF);
    tmp %= 1000;
    DrawUI_Bitmap_Ex(16+ 8,96,8,16, Font_8x16_O[2],0xFFFF);
    DrawUI_Bitmap_Ex(16+16,96,8,16, Font_8x16[tmp/100],0xFFFF);
    tmp %= 100;
    DrawUI_Bitmap_Ex(16+24,96,8,16, Font_8x16[tmp/10],0xFFFF); 
    tmp %= 10;
    DrawUI_Bitmap_Ex(16+32,96,8,16, Font_8x16[tmp],0xFFFF);       
    #else
    // UI_Draw_Title_Bluetooth_(36,0);
    UI_Draw_Title_Bluetooth_2();
    UI_Draw_Title();
    DrawUI_JPEG(0,8,ui_jpeg_uv);
    
    int hour = TimerHH();
    
    if ( config.time_format == 1 )
    {
        if ( hour > 11 )
        {
            hour -= 12;
        }
        
        if ( hour == 0 )
        {
            hour = 1;
        }
    }
    
    DrawUI_Bitmap_Ex( 36-10+2,44,11, 24, Font_11x18[hour/10], 0xFFFF);
    DrawUI_Bitmap_Ex( 47-10+2,44,11, 24, Font_11x18[hour%10], 0xFFFF);
    if ( update_flag & 0x1 )
    DrawUI_Bitmap_Ex( 58-10,44,11, 24, Font_11x18[10], 0xFFFF);
    DrawUI_Bitmap_Ex( 69-10-2,44,11, 24, Font_11x18[TimerMI()/10], 0xFFFF);
    DrawUI_Bitmap_Ex( 80-10-2,44,11, 24, Font_11x18[TimerMI()%10], 0xFFFF);
    
    for ( int i = 0; i < 0+15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j,COLORREF(100,100,100));
        }
    }
    
    for ( int i = 16; i < 16+15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j,COLORREF(100,100,100));
        }
    }

    for ( int i = 32; i < 32+15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j,COLORREF(100,100,100));
        }
    }

    for ( int i = 48; i < 48+15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j,COLORREF(100,100,100));
        }
    }

    for ( int i = 64; i < 64+15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j,COLORREF(100,100,100));
        }
    }        
    
    int uit_lvl = uit_get_level(UIT_i[0]);
    
    for ( int i = 16 * uit_lvl; i < 16 * uit_lvl + 15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j, color_map[uit_lvl]);
        }            
    }
    
    {
        DrawUI_JPEG(16 * uit_lvl+4,85,jpeg_ui_tangle);
    }
    
    #if 0
    DrawUI_JPEG(6,110-10,jpeg_ui_uv1);
    
    DrawUI_JPEG( 40, 106-10, bmp_cycle);        
    DrawUI_Bitmap_Exx( 40+3, 110-11, 26, 27, bmp_cycle_1, color_map[uit_lvl]);
    #else
    
    DrawUI_JPEG( (80-42)/2, 93,jpeg_ui_level[uit_lvl%5]);    

    #endif
    
    DrawUI_Bitmap_Exx( (80-11*2)/2,110-11+3+2,11, 24, Font_11x18[uit_lvl/10], 0x0000);
    DrawUI_Bitmap_Exx( (80-11*2)/2+11,110-11+3+2,11, 24, Font_11x18[uit_lvl%10], 0x0000); 
    
    switch ( uit_lvl )
    {
    case 0:  
        DrawUI_JPEG( 30, 140, jpeg_ui_ok);    
        break;
    case 1:
        DrawUI_JPEG( 6, 140, jpeg_ui_glass);  
        DrawUI_JPEG( 30, 140, jpeg_ui_ok);    
        DrawUI_JPEG( 51, 140, jpeg_ui_sunscreen);            
        break;
    case 2:
        DrawUI_JPEG( 6, 140, jpeg_ui_glass);  
        DrawUI_JPEG( 30, 140, jpeg_ui_hat);    
        DrawUI_JPEG( 51, 140, jpeg_ui_sunscreen);            
        break;
    case 3:
        DrawUI_JPEG( 6, 140, jpeg_ui_glass);  
        DrawUI_JPEG( 30, 140, jpeg_ui_hat);    
        DrawUI_JPEG( 51, 140, jpeg_ui_umbrella);             
        break;
    case 4:   
        DrawUI_JPEG( 6, 140, jpeg_ui_forbid);  
        DrawUI_JPEG( 30, 140, jpeg_ui_outside);    
        DrawUI_JPEG( 51, 140, jpeg_ui_tent);             
        break;
    default:
        break;
    }
    #endif
}

const unsigned char *weather_p[] = 
{
	Sunshine,        //晴
	Cloudy,          //多云
	Wet,             //下雨
	Snow,            //下雪  
	Smaze,           //霾
	Mote,            //扬尘  
};

const unsigned char *Temp_extent[] =
{
	Zero,
	One,
	Tow,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Unit_extent,     // ~
	Sub,            // -
	Cent,           //摄氏度
};

void ui_weather(void)
{
    int c_temp = fm.weather.ct & 0x7F;
    int h_temp = fm.weather.ht & 0x7F;
    int l_temp = fm.weather.lt & 0x7F;
	
    // 温度数值.
	
    if ( fm.weather.ct & 0x80 )
    {
        c_temp *= -1;
    }
    
    if ( fm.weather.ht & 0x80 )
    {
        h_temp *= -1;
    }

    if ( fm.weather.lt & 0x80 )
    {
        l_temp *= -1;
    }    
    
    //////////////////////////////////////////////////////////////////
    // 日期
    
    DrawUI_Bitmap_Exx( 4, 0,6, 16, Font_6x16[TimerMM()/10], 0xFFFF);
    DrawUI_Bitmap_Exx( 10,0,6, 16, Font_6x16[TimerMM()%10], 0xFFFF);
    DrawUI_Bitmap_Exx( 16+1,0,6, 16, Font_6x16[11], 0xFFFF);
    DrawUI_Bitmap_Exx( 22+2,0,6, 16, Font_6x16[TimerDD()/10], 0xFFFF);
    DrawUI_Bitmap_Exx( 28+2,0,6, 16, Font_6x16[TimerDD()%10], 0xFFFF);    
    
    // 蓝牙图标
	
    UI_Draw_Title_Bluetooth_2();
    // 电池图标
	
    UI_Draw_Title();
    // 天气图标
	
	DrawUI_JPEG((80-24)/2, 28, weather_p[fm.weather.tt]);
    
    // 当前温度
    if(c_temp >= 0)
	{
		if(c_temp < 10)
		{
			DrawUI_Bitmap_Exx( (80-22-8)/2+0, 65,22, 24, CurTemp_22x24[c_temp], 0x073F);
			DrawUI_JPEG((80-22-8)/2+22, 65, Temp_unit);
		}
		else
		{
			DrawUI_Bitmap_Exx( (80-22*2-8)/2+0, 65,22, 24, CurTemp_22x24[c_temp/10], 0x073F);   
			DrawUI_Bitmap_Exx( (80-22*2-8)/2+22,65,22, 24, CurTemp_22x24[c_temp%10], 0x073F);
			DrawUI_JPEG((80-22*2-8)/2+44, 65, Temp_unit);
		}
	}
	else
	{
		c_temp *= -1;
		if(c_temp < 10)
		{
			DrawUI_Bitmap_Exx( (80-22*2-8)/2+0, 65,22, 24, CurTemp_22x24[10], 0x073F);
			DrawUI_Bitmap_Exx( (80-22*2-8)/2+22, 65,22, 24, CurTemp_22x24[c_temp], 0x073F);
			DrawUI_JPEG((80-22*2-8)/2+44, 65, Temp_unit);
		}
		else
		{
			DrawUI_Bitmap_Exx( (80-22*3-8)/2+0, 65,22, 24, CurTemp_22x24[10], 0x073F);   
			DrawUI_Bitmap_Exx( (80-22*3-8)/2+22,65,22, 24, CurTemp_22x24[c_temp/10], 0x073F);
			DrawUI_Bitmap_Exx( (80-22*3-8)/2+44,65,22, 24, CurTemp_22x24[c_temp%10], 0x073F);     
			DrawUI_JPEG((80-22*3-8)/2+66, 65, Temp_unit);
		}
	
	}
    
    if ( (fm.weather.ht != 0xFF) && (fm.weather.lt != 0xFF) )
    {
        // 分割线

        for ( int x = 15; x < 65; x++ )
        {
            set_pixel( x, 96, 0xFFFF);
        }
    }
    
    // 最高气温，最低气温.
	
	if ( (fm.weather.ht != 0xFF) && (fm.weather.lt != 0xFF) )
    {
	if( h_temp >= 0 && l_temp >= 0 )
	{
		if(h_temp < 10)
		{
			if(l_temp < 10)
			{
				DrawUI_JPEG( (80-8*3-12)/2+0, 103, Temp_extent[l_temp]);
				DrawUI_JPEG( (80-8*3-12)/2+8, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*3-12)/2+16,103, Temp_extent[h_temp]);
				DrawUI_JPEG( (80-8*3-12)/2+24,102, Temp_extent[12]);
			}
			else
			{
				DrawUI_JPEG( (80-8*4-12)/2+0, 103, Temp_extent[l_temp/10]);
				DrawUI_JPEG( (80-8*4-12)/2+8, 103, Temp_extent[l_temp%10]);
				DrawUI_JPEG( (80-8*4-12)/2+16,103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*4-12)/2+24,103, Temp_extent[h_temp]);
				DrawUI_JPEG( (80-8*4-12)/2+32,102, Temp_extent[12]);		
			}				
		}
		else
		{
			if(l_temp < 10)
			{
				DrawUI_JPEG( (80-8*4-12)/2+0, 103, Temp_extent[l_temp]);
				DrawUI_JPEG( (80-8*4-12)/2+8, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*4-12)/2+16,103, Temp_extent[h_temp/10]);
				DrawUI_JPEG( (80-8*4-12)/2+24,103, Temp_extent[h_temp%10]);
				DrawUI_JPEG( (80-8*4-12)/2+32,102, Temp_extent[12]);
			}
			else
			{
				DrawUI_JPEG( (80-8*5-12)/2+0, 103, Temp_extent[l_temp/10]);
				DrawUI_JPEG( (80-8*5-12)/2+8, 103, Temp_extent[l_temp%10]);
				DrawUI_JPEG( (80-8*5-12)/2+16,103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*5-12)/2+24,103, Temp_extent[h_temp/10]);
				DrawUI_JPEG( (80-8*5-12)/2+32,103, Temp_extent[h_temp%10]);
				DrawUI_JPEG( (80-8*5-12)/2+40,102, Temp_extent[12]);		
			}			
		}
	}
    
	if ( h_temp >= 0 && l_temp < 0 )
	{
		l_temp *= -1;
		if(h_temp < 10)
		{			
			if(l_temp < 10)
			{
				DrawUI_JPEG( (80-8*4-12)/2+0, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*4-12)/2+8, 103, Temp_extent[l_temp]);
				DrawUI_JPEG( (80-8*4-12)/2+16, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*4-12)/2+24,103, Temp_extent[h_temp]);
				DrawUI_JPEG( (80-8*4-12)/2+32,102, Temp_extent[12]);
			}
			else
			{
				DrawUI_JPEG( (80-8*5-12)/2+0, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*5-12)/2+8, 103, Temp_extent[l_temp/10]);
				DrawUI_JPEG( (80-8*5-12)/2+16, 103, Temp_extent[l_temp%10]);
				DrawUI_JPEG( (80-8*5-12)/2+24, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*5-12)/2+32,103, Temp_extent[h_temp]);
				DrawUI_JPEG( (80-8*5-12)/2+40,102, Temp_extent[12]);		
			}				
		}
		else
		{
			if(l_temp < 10)
			{
				DrawUI_JPEG( (80-8*5-12)/2+0, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*5-12)/2+8, 103, Temp_extent[l_temp]);
				DrawUI_JPEG( (80-8*5-12)/2+16, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*5-12)/2+24,103, Temp_extent[h_temp/10]);
				DrawUI_JPEG( (80-8*5-12)/2+32,103, Temp_extent[h_temp%10]);
				DrawUI_JPEG( (80-8*5-12)/2+40,102, Temp_extent[12]);
			}
			else
			{
				DrawUI_JPEG( (80-8*6-12)/2+0, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*6-12)/2+8, 103, Temp_extent[l_temp/10]);
				DrawUI_JPEG( (80-8*6-12)/2+16, 103, Temp_extent[l_temp%10]);
				DrawUI_JPEG( (80-8*6-12)/2+24, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*6-12)/2+32,103, Temp_extent[h_temp/10]);
				DrawUI_JPEG( (80-8*6-12)/2+40,103, Temp_extent[h_temp%10]);
				DrawUI_JPEG( (80-8*6-12)/2+48,102, Temp_extent[12]);		
			}				
		}
	}
    
	if( h_temp < 0 && l_temp >= 0 )
	{
		h_temp *= -1;
		if(h_temp < 10)
		{	
			if(l_temp < 10)
			{
				DrawUI_JPEG( (80-8*4-12)/2+0, 103, Temp_extent[l_temp]);
				DrawUI_JPEG( (80-8*4-12)/2+8, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*4-12)/2+16, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*4-12)/2+24,103, Temp_extent[h_temp]);
				DrawUI_JPEG( (80-8*4-12)/2+32,102, Temp_extent[12]);	
			}
			else
			{		
				DrawUI_JPEG( (80-8*5-12)/2+0, 103, Temp_extent[l_temp/10]);
				DrawUI_JPEG( (80-8*5-12)/2+8, 103, Temp_extent[l_temp%10]);
				DrawUI_JPEG( (80-8*5-12)/2+16, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*5-12)/2+24, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*5-12)/2+32,103, Temp_extent[h_temp]);
				DrawUI_JPEG( (80-8*5-12)/2+40,102, Temp_extent[12]);
			}
		}
		else
		{
			if(l_temp < 10)
			{			
				DrawUI_JPEG( (80-8*5-12)/2+0, 103, Temp_extent[l_temp]);
				DrawUI_JPEG( (80-8*5-12)/2+8, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*5-12)/2+16, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*5-12)/2+24,103, Temp_extent[h_temp/10]);
				DrawUI_JPEG( (80-8*5-12)/2+32,103, Temp_extent[h_temp%10]);
				DrawUI_JPEG( (80-8*5-12)/2+40,102, Temp_extent[12]);
			}
			else
			{			
				DrawUI_JPEG( (80-8*6-12)/2+0, 103, Temp_extent[l_temp/10]);
				DrawUI_JPEG( (80-8*6-12)/2+8, 103, Temp_extent[l_temp%10]);
				DrawUI_JPEG( (80-8*6-12)/2+16, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*6-12)/2+24, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*6-12)/2+32,103, Temp_extent[h_temp/10]);
				DrawUI_JPEG( (80-8*6-12)/2+40,103, Temp_extent[h_temp%10]);
				DrawUI_JPEG( (80-8*6-12)/2+48,102, Temp_extent[12]);		
			}				
		
		}	
	}
	if(h_temp < 0 && l_temp < 0)
	{
		h_temp *= -1;
		l_temp *= -1;
		if(h_temp < 10)
		{			
			if(l_temp < 10)
			{
				DrawUI_JPEG( (80-8*5-12)/2+0, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*5-12)/2+8, 103, Temp_extent[l_temp]);
				DrawUI_JPEG( (80-8*5-12)/2+16, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*5-12)/2+24, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*5-12)/2+32,103, Temp_extent[h_temp]);
				DrawUI_JPEG( (80-8*5-12)/2+40,102, Temp_extent[12]);
			}
			else
			{
				DrawUI_JPEG( (80-8*6-12)/2+0, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*6-12)/2+8, 103, Temp_extent[l_temp/10]);
				DrawUI_JPEG( (80-8*6-12)/2+16, 103, Temp_extent[l_temp%10]);
				DrawUI_JPEG( (80-8*6-12)/2+24, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*6-12)/2+32, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*6-12)/2+40,103, Temp_extent[h_temp]);
				DrawUI_JPEG( (80-8*6-12)/2+48,102, Temp_extent[12]);		
			}				
		}
		else
		{
			if(l_temp < 10)
			{
				DrawUI_JPEG( (80-8*6-12)/2+0, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*6-12)/2+8, 103, Temp_extent[l_temp]);
				DrawUI_JPEG( (80-8*6-12)/2+16, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*6-12)/2+24, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*6-12)/2+32,103, Temp_extent[h_temp/10]);
				DrawUI_JPEG( (80-8*6-12)/2+40,103, Temp_extent[h_temp%10]);
				DrawUI_JPEG( (80-8*6-12)/2+48,102, Temp_extent[12]);
			}
			else
			{
				DrawUI_JPEG( (80-8*7-12)/2+0, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*7-12)/2+8, 103, Temp_extent[l_temp/10]);
				DrawUI_JPEG( (80-8*7-12)/2+16, 103, Temp_extent[l_temp%10]);
				DrawUI_JPEG( (80-8*7-12)/2+24, 103, Temp_extent[10]);
				DrawUI_JPEG( (80-8*7-12)/2+32, 103, Temp_extent[11]);
				DrawUI_JPEG( (80-8*7-12)/2+40,103, Temp_extent[h_temp/10]);
				DrawUI_JPEG( (80-8*7-12)/2+48,103, Temp_extent[h_temp%10]);
				DrawUI_JPEG( (80-8*7-12)/2+56,102, Temp_extent[12]);		
			}				
		}
	}
	
    }
	
    // 时间
    
    int hour = TimerHH();
	if ( config.time_format == 1 )
	{
		if ( hour > 11 )
		{
			hour -= 12;
		}
		
		if ( hour == 0 )
		{ 
			hour = 12; 
		}
	}

    DrawUI_Bitmap_Exx((80-12*5)/2-1,125-2,12,12, Time_12x12[hour/10],0xFDE0);
	DrawUI_Bitmap_Exx((80-12*5)/2+12,125-2,12,12, Time_12x12[hour%10],0xFDE0);
	DrawUI_Bitmap_Exx((80-12*5)/2+24,125-2,12,12, Time_12x12[10],0xFDE0);
	DrawUI_Bitmap_Exx((80-12*5)/2+36,125-2,12,12, Time_12x12[TimerMI()/10],0xFDE0);
	DrawUI_Bitmap_Exx((80-12*5)/2+48+1,125-2,12,12, Time_12x12[TimerMI()%10],0xFDE0);
    // 星期
	DrawUI_Bitmap_Exx( (80-34)/2, 140,34, 12, Week_34x12[TimerWK()%7], 0xFFFF);
}

unsigned char flag__ = 0;

void show_demo(void)
{
    switch( flag__ )
    {
    case 0:        
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x0000);
        flag__ = 1;
        break;
    case 1:    
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x2000);
        flag__ = 2;
        break;
    case 2:    
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x4000);
        flag__ = 3;
        break;
    case 3:    
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x6000);
        flag__ = 4;
        break;
    case 4:    
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x8000);
        flag__ = 5;
        break;
    case 5:    
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0xa000);
        flag__ = 6; 
        break;
    case 6:    
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0xc000);
        flag__ = 7;
        break;
    case 7:    
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0xe000);
        flag__ = 0;    
        break;    
    }  
}    

unsigned char show_weath_flag = 0;

unsigned long win_main_proc(unsigned long wid, unsigned long msg_type, 
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
		
		if ( win_ticks > 3 )
		{
			showLogoFlag = 0;
			fristBootFlag = 0;
		}
		
        #if 1
		if ( (win_ticks > 8) && (config.ui_style == 2) )				
		{
			lcd_display(0);
			return ( msg_type ^ WINDOW_UPDATE );
		}else{
            if ( (win_ticks > 6) && (config.ui_style != 2) )
            {
                lcd_display(0);
                return ( msg_type ^ WINDOW_UPDATE );                
            }
        }
        #else
        if ( win_ticks > 800 )
        {
            lcd_display(0);
            return ( msg_type ^ WINDOW_UPDATE );    
        }
        #endif
		
        DrawBackGroud();
		
		if ( showLogoFlag == 1 )
		{					
            if ( win_ticks > 1 )
            {
                if ( fristBootFlag == 1 )
                {
                    show_ui_version();
                }else{
                    show_ui_logo();
                }
            }			
		}else
		{	
            #if 1
            if ( show_weath_flag == 1 )
            {
                ui_weather();
            }else{
                if ( config.ui_style == 0 )
                {
                    show_main_ui_style_0();
                }
                else
                {              
                    show_uit_win();
                }	
            }
            #else
                show_demo();
            #endif
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
			lcd_display(1);
			msg_type |= WINDOW_UPDATE;
		}else{
            if ( (fm.weather.mm != TimerMM()) || (fm.weather.dd != TimerDD()) ) 
            {
                showLogoFlag = 0; // 显示Vxxx期间按键.
                show_weath_flag = 0;
                window_push(window_get(win_current));
                msg_type |= WINDOW_DISTORY;
            }
            else
            {	
                if( show_weath_flag == 1)
                {
                    window_push(window_get(win_current));
                    msg_type |= WINDOW_DISTORY;
                    show_weath_flag = 0;
                    config.ui_style = 0;
                }
                else 
                {
                    show_weath_flag = 1;
                }
                msg_type |= WINDOW_UPDATE;               
            }
        }
		
		return ( msg_type ^ WINDOW_TOUCH );
	}
	
	if ( msg_type & WINDOW_PRESS )
	{
		win_ticks = 0;
		
		if ( config.lcdEnable == 0 )
		{
			lcd_display(1);
			
			msg_type |= WINDOW_UPDATE;
			
			return ( msg_type ^ WINDOW_PRESS );
		}
	    
        if ( show_weath_flag == 0 )
        {
            config.ui_style++; config.ui_style %= 2;
    	}
        
		msg_type |= WINDOW_UPDATE;
		
		return ( msg_type ^ WINDOW_PRESS );
	}
	
	if ( msg_type & WINDOW_NOTIFY )
	{   
        if ( power.charge_flage == 1 )
        {
            return ( msg_type ^ WINDOW_NOTIFY );
        }
        
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
		selected = 0;
		
		window_pop(&win_current);
		
		return window[win_current].wproc( win_current, WINDOW_CREATE, 0, 0 );
	}

    return 0;
}

