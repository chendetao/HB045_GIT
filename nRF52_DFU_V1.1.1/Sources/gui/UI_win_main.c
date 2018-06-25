
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

#include "jpeg_decode.h"

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

#define COLORREF(a,b,c) ((a>>3)<<11)|((b>>2)<<5)|(c>>3))

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
    UI_Draw_Title_Bluetooth_(36,0);
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
            set_pixel(i,j,COLORREF(100,100,100);
        }
    }
    
    for ( int i = 16; i < 16+15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j,COLORREF(100,100,100);
        }
    }

    for ( int i = 32; i < 32+15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j,COLORREF(100,100,100);
        }
    }

    for ( int i = 48; i < 48+15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j,COLORREF(100,100,100);
        }
    }

    for ( int i = 64; i < 64+15; i++ )
    {
        for ( int j = (160-6)/2; j < (160-6)/2+6; j++ )
        {
            set_pixel(i,j,COLORREF(100,100,100);
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



void UI_Draw_Title_Bluetooth_2(void)
{
	if ( ble.isConnected == 0 )
	{		
		DrawUI_Bitmap_Rever(42,0,12,16,bmp_bluetooth[0],0x1F<<0);
	}else
	{
		DrawUI_Bitmap_Rever(42,0,12,16,bmp_bluetooth[1],0x1F<<0);
	}	
}
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
    
    // 分割线
	//DrawUI_Bitmap_Exx( 15,96,50, 1, dissever, 0xFFFF);
    for ( int x = 15; x < 65; x++ )
    {
        set_pixel( x, 96, 0xFFFF);
    }

    // 最高气温，最低气温.
	
	#if 1
	if(h_temp >= 0 && l_temp >= 0)
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
	if(h_temp >= 0 && l_temp < 0)
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
	if(h_temp < 0 && l_temp >= 0)
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
	#endif
	//时间

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
        draw_bitmap_from_flash( (80-72)/2, (160-48)/2, 72, 48, 0x200000+0x0000+0xf000);
        flag__ = 1;
        break;
    case 1:    
        draw_bitmap_from_flash( (80-53)/2, (160-72)/2, 53, 72, 0x200000+0x2000+0xf000);
        flag__ = 2;
        break;
    case 2:    
        draw_bitmap_from_flash( (80-53)/2, (160-72)/2, 53, 72, 0x200000+0x4000+0xf000);
        flag__ = 3;
        break;
    case 3:    
        draw_bitmap_from_flash( (80-58)/2, (160-48)/2, 58, 48, 0x200000+0x6000+0xf000);
        flag__ = 4;
        break;
    case 4:    
        draw_bitmap_from_flash( (80-64)/2, (160-64)/2, 64, 64, 0x200000+0x8000+0xf000);
        flag__ = 5;
        break;
    case 5:    
        draw_bitmap_from_flash( (80-65)/2, (160-56)/2, 65, 56, 0x200000+0xf000+0xf000);
        flag__ = 0;    
        break;    
    }  
}    

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
            if ( config.ui_style == 0 )
            {
				if(config.weatherlog == 1)
					ui_weather();
				else
					show_main_ui_style_0();
            } else if ( config.ui_style == 1 )
            {	
				if(config.weatherlog == 1)
					ui_weather();
				else
					show_main_ui_style_1(win_ticks);
            }
            else
            {              
				if ( config.weatherlog == 1 )
				{
					ui_weather();				
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
                config.weatherlog = 0;
                window_push(window_get(win_current));
                msg_type |= WINDOW_DISTORY;
            }
            else
            {	
                if(config.weatherlog == 1)
                {
                    window_push(window_get(win_current));
                    msg_type |= WINDOW_DISTORY;
                    config.weatherlog = 0;
                }
                else 
                {
                    config.weatherlog = 1;
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
	    
        config.ui_style++; config.ui_style %= 3;
    	
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

