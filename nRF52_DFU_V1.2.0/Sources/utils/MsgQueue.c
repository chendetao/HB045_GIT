/*******************************************************************************
* Copytight 2017 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : MsgQueue.c                                                        *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : File MsgQueue.c implements Message Buffer manage                *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017.03.02 1.0        build this moudle                   *
*                                                                              *
* Created : 2017.03.02                                                         *
* Last modified : 2017.03.02                                                   *
*                                                                              *
*******************************************************************************/
#include "string.h"
#include "OSAL.h"
#include "MsgQueue.h"

#define MAX_BODY_LEN  (256)  /* Message Body in total 1KB */
#define MAX_TITLE_LEN 30 /* Sender in total 30 bytes */

unsigned char title_buf[MAX_TITLE_LEN];
unsigned char body_buf[MAX_BODY_LEN];

MessageQ_Typdef mq = {
    .id = 0xFFFFFFFF,
    .title_len = 0,
    .title = title_buf,
    .body_len = 0,
    .body = body_buf,
	.notify_id = 0,
};

/**
 * void MessageClearAll(void);
 * @func  reset the Queue buffer
 * @return none
 */
void MessageClearAll(void)
{
    mq.id = 0xFFFFFFFF;
    mq.title_len = 0;
    mq.body_len = 0;
	mq.offset = 0;
    memset(title_buf, 0, MAX_TITLE_LEN );
    memset(body_buf, 0, MAX_BODY_LEN);
}

/**
 * int MessageBuildContent(unsigned char id, unsigned char length, const unsigned char *content);
 * @func  build the message Content buffer
 * @id  the identifier of message
 * @length the lenght of Content body
 * @content A pointer to the Content body encoded by unicode
 */
int MessageBuildContent(unsigned long id, unsigned long length, const unsigned char *content)
{
    unsigned long max;
    
    if ( id != mq.id )
    {
        memset( mq.body, 0, MAX_BODY_LEN );
        mq.body_len = 0;
		mq.offset = 0;		
    }
    
    max = MAX_BODY_LEN - mq.body_len;
    if ( length > max ) { length = max; }
    
    memcpy( mq.body+mq.body_len, content, length);
    mq.id = id;
    mq.body_len += length;
    
    return 0;
}

/**
 * int MessageBuildSender(unsigned char id, unsigned char length, const unsigned char *content);
 * @func  build the message sender buffer
 * @id  the identifier of message
 * @length the lenght of message body
 * @content A pointer to the message body encoded by unicode
 */
int MessageBuildSender(unsigned long id, unsigned long length, const unsigned char *content)
{
    unsigned long max;
    
    if ( id != mq.id )
    {
        memset( mq.title, 0, MAX_TITLE_LEN ); 
        mq.title_len = 0;
		mq.offset = 0;
    }
    
    max = MAX_TITLE_LEN - mq.title_len;
    if ( length > max ) { length = max; }
    memcpy( mq.title+mq.title_len, content, length );
    
    mq.id = id;
    mq.title_len += length;
       
    return 0;
}

volatile int error = 0;

bool MessageUpdatePage(void)
{
	if ( (mq.offset == mq.body_len) || (mq.body_len < (4*6*2)) )
	{
		mq.offset = 0; return true;
	}
	        
	mq.offset += mq.pageLength;
	
	if ( mq.offset == mq.body_len ) // last page
	{
		mq.offset = 0; return true;
	}	
	
	return false;
}

#include "UI_win_notify.h"
#include "Task_GUI.h"

void MessageTest(void)
{
	unsigned char buf[] = {0x00,0x31,0x00,0x32,0x00,0x33,0x00,0x34 };
	
	MessageBuildSender( 1, 8, buf );
	NOTICE_ADD(NOFIFY_ID_CALLING );	
}
