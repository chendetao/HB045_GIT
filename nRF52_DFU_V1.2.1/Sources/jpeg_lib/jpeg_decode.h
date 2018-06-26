#ifndef __JPEG_DECODE_H__
#define __JPEG_DECODE_H__

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;


union color_t
{
	uint16_t U16;
	uint8_t  U8[2];
};

extern void DrawUI_JPEG( int x, int y, const unsigned char *buf );
extern void DrawUI_JPEG_x( int x, int y, const unsigned char *buf );
extern void DrawUI_JPEG_xx( int x, int y, const unsigned char *buf );

#endif

