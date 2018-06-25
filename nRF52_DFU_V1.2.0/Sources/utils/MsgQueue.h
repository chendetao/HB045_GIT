/*******************************************************************************
* Copytight 2017 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : MsgQueue.h                                                        *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : File MsgQueue.h is the header of MsgQueue.c                     *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017.03.02 1.0        build this moudle                   *
*                                                                              *
* Created : 2017.03.02                                                         *
* Last modified : 2017.03.02                                                   *
*                                                                              *
*******************************************************************************/

#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#define NOTIFY_TYPE_CALL  		0
#define NOTIFY_TYPE_MSG   		1
#define NOTIFY_TYPE_QQ    		2
#define NOTIFY_TYPE_WX    		3
#define NOTIFY_TYPE_FACEBOOK 	4
#define NOTIFY_TYPE_WHATSAPP 	5
#define NOTIFY_TYPE_LINE        6
#define NOTIFY_TYPE_TT          7
#define NOTIFY_TYPE_INS         8
#define NOTIFY_TYPE_SKYPE       9 
#define NOTIFY_TYPE_ALL        10

#define NOTIFY_TYPE_INVALID  0xFF

#include <stdbool.h>

typedef struct
{
    unsigned long id;
    unsigned long title_len;
    unsigned long body_len;
	unsigned long offset;
	unsigned long pageLength;
    unsigned char *title;	
    unsigned char *body;
	unsigned char type;
	unsigned char push_by_ios;
	unsigned long current_len;
	unsigned char notify_id;
}MessageQ_Typdef;

extern MessageQ_Typdef mq;

extern int MessageBuildSender(unsigned long id, unsigned long length, const unsigned char *content);
extern int MessageBuildContent(unsigned long id, unsigned long length, const unsigned char *content);
extern void MessageClearAll(void);
extern bool MessageUpdatePage(void);

extern void MessageTest(void);

#endif
