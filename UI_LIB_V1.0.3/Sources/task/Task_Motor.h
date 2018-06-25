#ifndef __TASK_MOTOR_H__
#define __TASK_MOTOR_H__

#include "Board.h"
#include "nrf_gpio.h"

#if 1
#define MOTOR_INIT() do\
{\
	nrf_gpio_cfg_output(MOTOR_PWR_PIN);\
	MOTOR_ENABLE(false);\
}while(0)
#else
#define	MOTOR_INIT()
#endif

#if 1
#define MOTOR_ENABLE(enable) do \
{\
	if ( enable )\
		nrf_gpio_pin_set(MOTOR_PWR_PIN);\
	else \
		nrf_gpio_pin_clear(MOTOR_PWR_PIN);\
}while(0)

#else
#define MOTOR_ENABLE(enable)
#endif

#define TASK_MOTOR_BLE_CONNECT_EVT   	0x00000001
#define TASK_MOTOR_ACTION_DONE_EVT   	0x00000002
#define TASK_MOTOR_BMA_DONE_EVT      	0x00000004
#define TASK_MOTOR_VIBORATE_EVT      	0x00000008
#define TASK_MOTOR_ACTION_NOTIFY_EVT 	0x00000010
#define TASK_MOTOR_ACTION_APP_MSG_EVT 	0x00000020
#define TASK_MOTOR_TEST_EVT      	    0x00000040

extern unsigned long taskMotorTaskId;

extern unsigned long taskMotor( unsigned long task_id, unsigned long events );
extern void taskMotorInit( unsigned long task_id );

#endif
