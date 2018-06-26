#ifndef __UI_DRAW_H__
#define __UI_DRAW_H__

#include "LCD_ST7735.h"


#define USING_OLD_CONFIG  		0


#define ALGIN_LEFT 				0
#define ALGIN_RITH 				1
#define ALGIN_CENTER 			2

extern unsigned short frame_buffer[160][80];

#if 0
#define set_pixel( x, y, color) do\
{\
	if ( x < LCD_W && y < LCD_H )\
	 frame_buffer[y][x] = color;\
}while(0)
#else
extern void set_pixel( int x, int y, short color);
#endif

extern void UI_Draw_Font_10x24( int x, int y, int idx );
extern void draw_bitmap(int fx, int fy, int width, int height, const unsigned \
	char *bitmap );
extern void DrawUI_Bitmap( int x, int y, int w, int h, \
	const unsigned char *bitmap,unsigned long F, unsigned long B);
extern void DrawUI_Bitmap_Extend( int x, int y, int w, int h, \
	const unsigned char *bitmap,unsigned long F, unsigned long B);
extern void DrawUI_Bitmap_Ex( int x, int y, int w, int h, \
	const unsigned char *bitmap,unsigned long F);
extern void DrawUI_Bitmap_Rever( int x, int y, int w, int h, \
	const unsigned char *bitmap,unsigned long C);

extern void UI_Draw_Font_8x12( int x, int y, int idx );
extern void UI_Draw_Val( int x, int y, int value);
extern void UI_Draw_BPM(int x, int y, int idx);
extern void UI_Draw_HR_Pressure(int x, int y, int idx);
extern void UI_Draw_Week(int x, int y, int idx);
extern void UI_Draw_Week_old(int x, int y, int idx);
extern void UI_Draw_AM( int x, int y, int idx);
extern void UI_Draw_pedo_hr_old( int x, int y, int idx);
extern void UI_Draw_Vline( int fx, int fy, int width, int height, int color);
extern void UI_Draw_Battery(int x, int y, int level, int color);
extern void draw_bitmap_from_flash(int fx, int fy, int width, int height, unsigned long addr );

extern void draw_line(int x1, int y1, int x2, int y2, unsigned short color);

extern void DrawUI_Bitmap_Exx( int x, int y, int w, int h,\
	const unsigned char *bitmap, unsigned short F );

extern void DrawUI_Bitmap_Ex( int x, int y, int w, int h, \
	const unsigned char *bitmap,unsigned long F);

#endif
