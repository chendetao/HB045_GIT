/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : OSAL_TaskDemo.c                                                   *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : A simple Task instance for OSAL                                 *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017-3.30  1.0        build this moudle                   *
*                                                                              *
*                                                                              *
* Created : 2017-3-30                                                          *
* Last modified : 2017.3.30                                                    *
*******************************************************************************/

#include "OSAL_TaskDemo.h"
#include "OSAL.h"
#include "OSAL_Task.h"
#include "OSAL_Timer.h"


unsigned long taskDemoTaskId;

unsigned long taskDemo( unsigned long task_id, unsigned long events )
{
	if ( events  &  DEMO_START_EVENT )
	{        
		return ( events ^ DEMO_START_EVENT );
	}
	
	if ( events & DEMO_STOP_EVENT )
	{
		return ( events ^ DEMO_STOP_EVENT );
	}
	
	return ( 0 );
}

void taskDemoInit( unsigned long task_id )
{
	taskDemoTaskId = task_id;
	
    osal_set_event(task_id, DEMO_START_EVENT );
}

