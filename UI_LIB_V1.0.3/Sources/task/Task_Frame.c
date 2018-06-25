#include "OSAL.h"
#include "OSAL_Task.h"
#include "OSAL_Timer.h"
#include "Task_Frame.h"
#include "LCD_ST7735.h"


unsigned long taskFrameTaskId;

void taskFrameInit(unsigned long task_id)
{
    taskFrameTaskId = task_id;
}

/**
 * 1，由于同步整个FrameBuffer的内容将花费过长的时间，可能会导致蓝牙
 *    连接事件得不到处理，所以此处将FrameBuffer分为4次同步，使得OSAL有
 *    机会去执行更为紧急的任务.
 * 2，同时，需要在OLED屏幕上绘图的任务可以刷新指定的区域，减少没有必要
 *    的绘图时间，提升效率。
 */

unsigned long taskFrame( unsigned long task_id, unsigned long events )
{
    if ( events & FRAME_TASK_SYNC_FRAME_EVT_0 )
    {
        do_sync_frame(0, 20);
        
        events |= FRAME_TASK_SYNC_FRAME_EVT_1;
        
        return ( events ^ FRAME_TASK_SYNC_FRAME_EVT_0 );
    }    
    
    if ( events & FRAME_TASK_SYNC_FRAME_EVT_1 )
    {
        do_sync_frame(20, 40);
        
        events |= FRAME_TASK_SYNC_FRAME_EVT_2;
        
        return ( events ^ FRAME_TASK_SYNC_FRAME_EVT_1 );
    }
    
    if ( events & FRAME_TASK_SYNC_FRAME_EVT_2 )
    {
        do_sync_frame(40,60);
        
        events |= FRAME_TASK_SYNC_FRAME_EVT_3;
        
        return ( events ^ FRAME_TASK_SYNC_FRAME_EVT_2 );
    }

    if ( events & FRAME_TASK_SYNC_FRAME_EVT_3 )
    {
        do_sync_frame(60, 80);
        
        events |= FRAME_TASK_SYNC_FRAME_EVT_4;
        
        return ( events ^ FRAME_TASK_SYNC_FRAME_EVT_3 );
    } 
    
    if ( events & FRAME_TASK_SYNC_FRAME_EVT_4 )
    {
        do_sync_frame(80,100);
        
        events |= FRAME_TASK_SYNC_FRAME_EVT_5;
        
        return ( events ^ FRAME_TASK_SYNC_FRAME_EVT_4 );
    }

    if ( events & FRAME_TASK_SYNC_FRAME_EVT_5 )
    {
        do_sync_frame(100, 120);
        
        events |= FRAME_TASK_SYNC_FRAME_EVT_6;
        
        return ( events ^ FRAME_TASK_SYNC_FRAME_EVT_5 );
    } 

    if ( events & FRAME_TASK_SYNC_FRAME_EVT_6 )
    {
        do_sync_frame(120,140);
        
        events |= FRAME_TASK_SYNC_FRAME_EVT_7;
        
        return ( events ^ FRAME_TASK_SYNC_FRAME_EVT_6 );
    }

    if ( events & FRAME_TASK_SYNC_FRAME_EVT_7 )
    {
        do_sync_frame(140, 160);
        
        return ( events ^ FRAME_TASK_SYNC_FRAME_EVT_7 );
    }    
    
    return 0;
}
