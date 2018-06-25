/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : OSAL_Task.c                                                       *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : Operation System Abstruct Layer Interface. (Task implementation)*
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017-3.30  1.0        build this moudle                   *
*                                                                              *
*                                                                              *
* Created : 2017-3-30                                                          *
* Last modified : 2017.3.30                                                    *
*******************************************************************************/

/**
 * 
 * V1.0
 */
#include "string.h" 
 
#include "OSAL_Task.h"
#include "OSAL.h"

#include "Task_Ble.h"
#include "Task_Flash.h"
#include "Task_Upload.h"
#include "Task_GUI.h"
#include "Task_Step.h"
#include "Task_Touch.h"
#include "Task_Battery.h"
#include "Task_Hr.h"
#include "Task_Motor.h"
#include "Task_Finder.h"
#include "Task_Store.h"
#include "Task_Frame.h"

/**
 * OSAL Event Handler(Event Processor), your task event processor function
 * must place on here.
 */
 
const pTaskEventHandlerFn tasksArr[] = 
{	  	 
	taskTouch,
    taskGUI,
	taskFrame,
	taskMotor,	
};

unsigned long taskEventsTable[SIZEOF(tasksArr)];
const unsigned long tasksCnt = SIZEOF(tasksArr);

/**
 * OSAL Compoments Initialize. your task initialization function
 * must place on here.
 */
void osal_init_tasks( void )
{
	unsigned long task_id = 0;

	memset( taskEventsTable, 0, sizeof(taskEventsTable) );
	
    /**
     * The sequence of initalization functions must be
     * in the same order ad the event handler.
     */  
	taskTouchInit( task_id++ );

    taskGUIInit( task_id++ );
		
	taskFrameInit( task_id++ );
	
	taskMotorInit( task_id++ );	
}
