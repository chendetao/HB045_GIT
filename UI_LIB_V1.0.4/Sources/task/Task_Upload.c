#include "string.h"
#include "time.h"
#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_Step.h"
#include "Task_Upload.h"
#include "Task_Flash.h"

#include "ClockTick.h"
#include "btprotocol.h"
#include "UserSetting.h"
#include "Log.h"

struct Queue q;
unsigned char qbuf[2048/QUEUE_ITEM_SIZE][QUEUE_ITEM_SIZE];

/*******************************************************************************
 *                                                                             *
 *  从Flash加载历史数据到内存队列中,以便快速发送,避免频繁操作Flash             *
 *                                                                             *
 ******************************************************************************/

int store_queue( int identifier, struct Queue *q, int length)
{
	if ( q->lock == 1 )  /* 队列中包含有数据,无法继续加载，稍后重试  */
	{
		return (QUEUE_LOCKED);
	}
	
	if ( fmc.lock == 1 ) /* Flash正在被lock,无法操作，稍后重试 */
	{
		return (QUEUE_LOCKED);
	}
	
	if ( fm.erea[identifier].items == 0 )
	{
		return (QUEUE_EMPTY);
	}
	
	q->lock  = 1;
	q->front = q->tail = 0;
	q->items = 0;
	
	/**
	 * 将所有数据加载到队列中.
	 */
	while( (fm.erea[identifier].items != 0) && (q->items < 128) )
	{
		flash_memory_get( identifier, qbuf[q->tail], length, false);
		q->tail++;
		q->items++;
	}
	
	/**
     * 数据加载完成之后，更新信息控制块.	
	 */
	osal_set_event( taskFlashTaskId, TASK_FLASH_UPDATE_EVT );
	
	return 0;
}

/*******************************************************************************
 *                                                                             *
 *  从RAM队列中取一条数据,数据取完成之后unlock                                 *
 *                                                                             *
 ******************************************************************************/

int load_queue( int identifier, struct Queue *q, unsigned char *buf, int length )
{	
	if ( q->items > 0 )
	{
		memcpy( buf, qbuf[q->front], length );
		
		q->front++;
		q->items--;
	}else{
		return (QUEUE_EMPTY);
	}
	
	/** 取完成最后一条数据之后,队列空，则解锁写入操作 */
	if ( q->items == 0 )
	{
		q->lock  = 0;
	}
	
	return 0;
}

/*******************************************************************************
 *                                                                             *
 *  蓝牙断开后将Queue重置,避免死锁                                             *
 *                                                                             *
 ******************************************************************************/

int reset_queue( void )
{
	q.front = q.tail = 0;
	q.items = 0;
	q.lock  = 0;
	
	return 0;
}
	
unsigned long taskUploadTaskId;

unsigned long taskUpload( unsigned long task_id, unsigned long events )
{
	/**
	* 复位队列数据(连接异常时)
	 */
	if ( events & TASK_UPLOAD_RESET_QUEUE_EVT )
	{
		reset_queue();     
		
		return ( events ^ TASK_UPLOAD_RESET_QUEUE_EVT );
	}
	
    return 0;
}

void taskUploadInit( unsigned long task_id )
{
	taskUploadTaskId = task_id;
}

