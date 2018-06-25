#include "QR_Encode.h"
#include "QR_Utils.h"

int QR_Encode_Create( unsigned char *text )
{
    int Level 	   = 0;
    int Ver    = 1;
    int AutoExtent = 0;
    int MaskingNo  = 0;

    bool ret;

    ret = encoder.EncodeData( &encoder, Level, Ver, AutoExtent, MaskingNo, text, 0);    

    if ( ret != true )
    {
        return -1;
    }
	
	return 0;
}

int QR_Copy_Image( unsigned short fb[160][80] )
{
    int symbleSize = encoder.m_nSymbleSize;
    
    for ( int y = 0; y < 160; y++)
	for ( int x = 0; x < 80; x++ )
		fb[y][x] = 0X6B6D;

	#if 0
	
	for ( int y = 0; y < 64; y++ )
	for ( int x = 0; x < 24; x++ )
		fb[y][x] = 0x00;
	
	for ( int y = 0; y < 64; y++ )
	for ( int x = 72; x < 96; x++ )
		fb[y][x] = 0x00;

	for ( int y = 0; y < 8; y++ )
	for ( int x = 0; x < 96; x++ )
		fb[y][x] = 0x00;

	for ( int y = 56; y < 64; y++ )
	for ( int x = 0; x < 96; x++ )
		fb[y][x] = 0x00;	
		
	#endif
	
    for ( int y = 0; y < 160; y++ )
    for ( int x = 0; x < 80; x++ )
    {
		if ( ( (y < (symbleSize)) ) && (x < symbleSize) )
		if ( encoder.m_byModuleData[y][x] )
		{					
			/** Stretch 3 times */
			
			fb[(y*3)+0+48][x*3+0+8] = 0x0;
			fb[(y*3)+0+48][x*3+1+8] = 0x0;
			fb[(y*3)+0+48][x*3+2+8] = 0x0;
			
			fb[(y*3)+1+48][x*3+0+8] = 0x0;
			fb[(y*3)+1+48][x*3+1+8] = 0x0;
			fb[(y*3)+1+48][x*3+2+8] = 0x0;	

			fb[(y*3)+2+48][x*3+0+8] = 0x0;
			fb[(y*3)+2+48][x*3+1+8] = 0x0;
			fb[(y*3)+2+48][x*3+2+8] = 0x0;			
		}
		
		if ( (y >= symbleSize) && (x == symbleSize) )
		{
			break;
		}
    }

	return 0;    
}
