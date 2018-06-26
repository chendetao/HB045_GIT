
#include <string.h>
#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_Finder.h"
#include "UserSetting.h"
#include "btprotocol.h"
#include "Window.h" 
#include "UI_win_notify.h"

unsigned long taskFinderTaskId;
Finder finder = {.state = FIND_PHONE_STATE_STOP, .isFound = 0, .phoneFindMe = 0,};
unsigned char buffer[20];

unsigned long taskFinder( unsigned long task_id, unsigned long events )
{
   if ( events & FIND_HONE_TASK_STOP_EVT )
   {
       /* 停止 FIND_HONE_TASK_START_EVT 定时器 */
       osal_stop_timerEx( task_id, FIND_HONE_TASK_START_EVT );

       /* 设备已经停止查找手机指令 */
       
       if ( ble.isConnected == 1)  
       {
           memset(buffer, 0, sizeof(buffer));
           
           buffer[0] = 0xFC;
           buffer[1] = 0x10;
           buffer[2] = 0x01;
           buffer[3] = 0x00;
           
		   bt_protocol_tx ( buffer, sizeof(buffer));
       }
                         
      return 0;  /* 这样可以使得本任务的所有事件全部清除掉 */
   }
    
   if ( events & FIND_HONE_TASK_START_EVT )
   {
       if ( ble.isConnected != 1)  /* 蓝牙不连接时，延时一秒再执行 */
       {
           osal_start_timerEx( task_id, FIND_HONE_TASK_START_EVT, 1000); 
           
           return ( events ^ FIND_HONE_TASK_START_EVT );
       }
       
       /* 已经停止查找了 */
       if ( finder.state == FIND_PHONE_STATE_STOP )
       {
           return ( events ^ FIND_HONE_TASK_START_EVT );
       }
       
       /* 发送查找手机指令 */
       
       memset(buffer, 0, sizeof(buffer));
       
       buffer[0] = 0xFC;
       buffer[1] = 0x10;
       buffer[2] = 0x01;
       buffer[3] = 0x01;
       
       bt_protocol_tx ( buffer, sizeof(buffer)); 
       
       return ( events ^ FIND_HONE_TASK_START_EVT );
   }
   
   if ( events & FINDPHONE_TASK_FINDME_START_EVT )
   {
       /* 手机开始查找 */
       
       NOTICE_ADD( NOTIFY_ID_FINDME );
       
       return ( events ^ FINDPHONE_TASK_FINDME_START_EVT );
   }
   
   if ( events & FINDPHONE_TASK_FINDME_STOP_EVT )
   {
       /* 手机停止查找 */
       
       finder.cancelFindMe = 1; /* 标记为手机取消查找，touch一下退出通知 */
       
       ui_window_manager( WINDOW_TOUCH, MAKE_MESSAGE(WIN_MSG_KEY_TOUCH), 0);
       
       return ( events ^ FINDPHONE_TASK_FINDME_STOP_EVT );
   }
   
   if ( events & FINDPHONE_TASK_FINDME_RESPONSE_EVT )
   {
       /* 用户触摸按键时，发送一个确认到app */
       
       memset(buffer, 0, sizeof(buffer));
       
       buffer[0] = 0x10;
       buffer[1] = 0x00;
       buffer[2] = 0x00;
       buffer[3] = 0x00;
       
       bt_protocol_tx ( buffer, sizeof(buffer));         
       
       return ( events ^ FINDPHONE_TASK_FINDME_RESPONSE_EVT );
   }
   	
	return 0;
}

void taskFinderInit( unsigned long task_id )
{
	taskFinderTaskId = task_id;
}
