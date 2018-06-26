#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_Motor.h"
#include "Board.h"
#include "UserSetting.h"

void motor_disable(void)
{
	MOTOR_ENABLE(false);
}

unsigned long taskMotorTaskId;

unsigned long taskMotor( unsigned long task_id, unsigned long events )
{
    return 0;
}

void taskMotorInit( unsigned long task_id )
{
	taskMotorTaskId = task_id;
	
	MOTOR_INIT();	
}
