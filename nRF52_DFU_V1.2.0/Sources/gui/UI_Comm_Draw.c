
#include "UI_comm_icon.h"
#include "UI_Comm_Draw.h"
#include "UI_Draw.h"

void show_val( int x, int y, unsigned long value, unsigned long mode )
{
	if ( (mode & 0x3) == MODE_ALIGN_CENTER )
	{
		int count;
		
		if ( value > 99999 ){ value = 99999; }
		
		if ( value > 9999 ){
			count = 5;
		}else if ( value > 999 ){
			count = 4;
		}else if ( value > 99 ){
			count = 3;
		}else if ( value > 9 ){
			count = 2;
		}else{
			count = 1;
		}
		
		int witdth = x + (96-x - (count * 9))/2;
		int ex = witdth + (count-1) * 9;
		do
		{
			draw_bitmap( ex, 29, 8, 10, gImage_00[value%10]);
			value /= 10; ex -= 9;
		}while( value > 0 );
	}
}
