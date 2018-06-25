#include "string.h"
#include <time.h>

#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_Step.h"
#include "Task_Upload.h"
#include "Task_Flash.h"
#include "Task_Store.h"
#include "Task_Ble.h"

#include "ClockTick.h"
#include "btprotocol.h"
#include "UserSetting.h"
#include "Log.h"

unsigned long taskStoreTaskId;

unsigned long taskStore (unsigned long task_id, unsigned long events )
{

	
	return 0;
}

void taskStoreInit( unsigned long task_id )
{
	taskStoreTaskId = task_id;
}
