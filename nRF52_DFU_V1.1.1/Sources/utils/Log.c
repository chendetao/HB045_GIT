/**************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved              *
*                                                                         *
* Filename : Log.c                                                        * 
* Author : Wen Shifang                                                    *
* Version : 1.0                                                           *
*                                                                         *
* Decription : Log system implementation.                                 *
* Function-list:                                                          *
* History:                                                                *
*       <author>     <time>     <version>  <desc>                         *
*       Wen Shifang  2017-4.05  1.0        build this moudle              *
*                                                                         *
* Created : 2017-4-5                                                      *
* Last modified : 2017.4.5                                                *
***************************************************************************/

#include "OSAL_Timer.h"
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

#include "pstorage.h"
#include "nrf_error.h"

#include "Log.h"

extern int ble_write_bytes( unsigned char *buf, int length );

int Log_write_to_flash(unsigned char *log, int len);
int Log_write_to_ble(unsigned char *log, int len);
int Log_write_to_uart(unsigned char *log, int len);

struct LogIf
{
	unsigned int magic;
	unsigned int w_offset;
	unsigned int r_offset;
	unsigned int override;
};

struct LogIf logIf;
unsigned int logoutputEnable = 1;

unsigned char logbuf[16];

#include <time.h>
#include "ClockTick.h"
#include "Task_Upload.h"
#include "OSAL.h"

int Log_Printf( int fd, const char *fmt, ... )
{
    char buf[64];
	int length;
    
	va_list ap;
	
	if ( logoutputEnable == 0 )
	{
		 return 0;
	}
	
	va_start( ap, fmt );
	length = vsnprintf( buf, sizeof(buf), fmt, ap );
	va_end(ap);
	
	//
		struct tm stm; 
		unsigned long msTicks;
		stm.tm_year = 2000 + TimerYY() - 1900;
		stm.tm_mon  = TimerMM();
		stm.tm_mday = TimerDD();
		stm.tm_hour = TimerHH();
		stm.tm_min  = TimerMI();
		stm.tm_sec  = TimerSE();  
	
		msTicks = mktime(&stm); 
		logbuf[0] = (msTicks >> 24) & 0xFF;
		logbuf[1] = (msTicks >> 16) & 0xFF;
		logbuf[2] = (msTicks >> 8) & 0xFF;
		logbuf[3] = (msTicks >> 0) & 0xFF;
	
		memcpy( logbuf +4, buf, (length > 16)?(16):(length));
		
		// osal_set_event ( taskUploadTaskId, TASK_UPLOAD_SAVE_LOG_EVT );
		
		return 0;
	
	//
}

int Log_write_to_flash( unsigned char *log, int len)
{	
    return 0;
}

int Log_read_flash_next( unsigned char *buf, int len )
{
	return 0;
}

int Log_write_to_ble( unsigned char *log, int len)
{
	unsigned char buf[20];
	
	buf[0] = 0x0F;
	buf[1] = 0x0A;
	buf[2] = 0x03;
	buf[4] = 0x00;
	
	memcpy(buf+4, log, (len>20)?(20):(len));
	
	ble_write_bytes(buf, len+4);
	ble_write_bytes(buf, len+4);
	
	return 0;
}

int Log_write_to_uart( unsigned char *log, int len)
{
	return 0;
}


