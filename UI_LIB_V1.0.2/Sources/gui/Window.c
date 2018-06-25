
#include <string.h>
#include <stdbool.h>
#include "OSAL.h"
#include "UI_win_main.h"
#include "UI_win_pedometer.h"
#include "UI_win_info.h"
#include "UI_win_hr.h"    
#include "UI_win_sleep.h"
#include "UI_win_shutdown.h"
#include "UI_win_findphone.h"
#include "UI_win_notify.h"
#include "UI_win_camera.h"
#include "UI_win_sport.h"
#include "Window.h"

#include "Task_GUI.h"

struct window_struct window[] = 
{
    [0] = {0, 0, 0, 0, 1, win_main_proc},
};

unsigned long win_current;
unsigned short win_ticks;
const unsigned long winCnt = SIZEOF(window);

unsigned long win_stack[4];
unsigned char current_idx;
unsigned long win_show_flag;  /* Flag show window or not */

void window_init(void)
{
	win_current = 0;
	win_ticks   = 0;
	current_idx = 0;
	memset(win_stack, 0, sizeof(win_stack));
	
	window[win_current].msg_type = WINDOW_CREATE;
}

void window_push( unsigned long wid )
{
	win_stack[current_idx++] = wid;
}

int window_pop( unsigned long *wid )
{
	if ( current_idx > 0 )
	{
		*wid = win_stack[--current_idx];
		return 0;
	}
	
	*wid = 0; /** Swith to identifier '0' if error occur.  */
	return (-1);
}

void window_tack_reset(void)
{
	current_idx = 0;
	
	memset( win_stack, 0, sizeof(win_stack));
}

unsigned long window_get( unsigned long win_current )
{
	unsigned long idx = win_current;
	
	while ( 1 )
	{
		idx = (idx + 1) % SIZEOF(window);
		if ( ( (win_show_flag >> idx ) & 0x1)
			&& (window[idx].flag == 1) )
		{
			 break;
		}
	}
	
	return idx;
}

int window_show_set( unsigned long win_id, int show )
{
	if ( win_id > SIZEOF(window) )
	{
		return (-1);
	}

	if ( ( !show ) && !(win_show_flag &= ~(0x1<<win_id)) )
	{
		/* Keep at least one window to display */
		return (-1);
	}
	
	if ( show )
	{
		win_show_flag |= (0x1<<win_id);
	}else{
		win_show_flag &= ~(0x1<<win_id);
	}
	
	return 0;
}

/**
 * 窗口ID映射表:
 * app_id 协议定义的窗口ID
 * dev_id 本地定义的窗口ID
 * 注意：只有在下表中有对应关系的窗口才可以控制.
 * 不存在对应关系的,操作不会生效,但不会影响程序逻辑.
 */
const struct window_map wmap[] = 
{
	 {0,0},
	 {1,1},
	 {2,2},
	 {3,3},
	 {4,4},
	 {5,9},
	 {6,8},
	 {7,7},
	 {9,5},
};

int win_identifier_map(int app_id)
{
	for ( int i = 0; i < SIZEOF(wmap); i++ )
	{
		if ( app_id == wmap[i].app_id )
		{
			 return wmap[i].dev_id;
		}
	}
	
	return (-1);
}

/**
 * int window_set(unsigned long enable, const unsigned char *pVal )
 * 设置窗口显示/隐藏
 * enable : 16位控制位,控制16个窗体的显示与否,实际只有13位有效.
 * pValue[x]:字节最高位表示有效位,指定本字节是否有效. 低7位为窗口ID
 */
int window_set(unsigned long enable, const unsigned char *pVal )
{
	int dev_id;
	for ( int i = 0; i < 13; i++ )
	{
		if ( pVal[i] & 0x80 )
		{
			/**
			 * MAP the protocol window id to local window id
			 */
			
			dev_id = win_identifier_map(pVal[i]&0x3F);
			if ( dev_id != -1){
				window_show_set( dev_id, (enable>>i)&0x1);
			}				
		}
	}
	
	return 0;
}

int ui_window_manager(unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p)
{
	window[win_current].msg_type = msg_type;
	window[win_current].param = msg_param;
	window[win_current].arg = p;
	
	osal_set_event( taskGUITaskId, TASK_GUI_UI_EVT );
    
    return 0;
}

int ui_window_set_param( unsigned long win_id, unsigned long msg_param )
{
	window[win_id].param = msg_param;
	return 0;
}
