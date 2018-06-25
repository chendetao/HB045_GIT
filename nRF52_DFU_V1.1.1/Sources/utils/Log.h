/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : Log.h                                                             * 
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : Log system implementation.                                      *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017-4.05  1.0        build this moudle                   *
*                                                                              *
*                                                                              *
* Created : 2017-4-5                                                           *
* Last modified : 2017.4.5                                                     *
*******************************************************************************/

#ifndef __LOG_H__
#define __LOG_H__

#define LOG_FD_FLASH  	0
#define LOG_FD_UART   	1
#define LOG_FD_BLE    	2

#define MAX_LOG_ITEM   64

extern unsigned int logoutputEnable;

extern unsigned char logbuf[16];

extern int Log_Init(void);
extern int Log_Printf( int fd, const char *fmt, ... );
extern int Log_read_flash_next( unsigned char *buf, int len );
extern int Log_write_to_ble( unsigned char *log, int len);

#endif

