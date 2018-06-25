#ifndef __TASKSTEP_H__
#define __TASKSTEP_H__

#define STEP_TASK_DECT_EVT		    		0x00000001
#define TASK_STEP_BMA_INIT_EVT          	0x00000002
#define TASK_STEP_UPLOAD_PEDOMETER_EVT  	0x00000004
#define TASK_STEP_CLEAR_EVT             	0x00000008
#define TASK_STEP_SED_INIT_EVT          	0x00000010
#define TASK_STEP_SED_NOTI_EVT              0x00000020
#define TASK_STEP_HR_COMPENT_EVT            0x00000040
#define TASK_STEP_UPLOAD_ENABLE_EVT         0x00000080
#define TASK_STEP_BSTOTW_EVT                0x00000100
#define TASK_STEP_CLEAR_ALL_EVT             0x00000200
#define TASK_STEP_SLEEP_ACTION_EVT          0x00000400
#define TASK_STEP_SLEEP_START_EVT           0x00000800 
#define TASK_STEP_SLEEP_DONE_EVT            0x00001000
#define TASK_STEP_BMA_RE_INIT_EVT           0x00002000
#define TASK_STEP_BSTOTW_ENABLE_EVT         0x00004000
#define TASK_STEP_HFLAG_CLEAR_EVT           0x00008000

#define STEP_TASK_SLEEP_DETECT_TICK_EVT     0x00010000
#define STEP_TASK_CLEAR_SLEEP_EVT           0x00020000
#define STEP_TASK_SLEEP_DETECT_EVT          0x00040000
#define STEP_TASK_NOTIFY_EVT                0x00080000
#define STEP_TASK_TEST_EVT                  0x00100000

#define STEP_TASK_UPLOAD_WAVE_EVT           0x00200000
#define STEP_TASK_INT_COMFIRM_EVT           0x00400000
#define TASK_STEP_MONITOR_COMFIRM_EVT       0x00800000
#define TASK_STEP_MONITOR_COMFIRM_EVT_1     0x10000000
#define TASK_STEP_UPLOAD_FAST_WAVE_EVT      0x20000000

#define STEP_TASK_NOTIFY_SLEEP_TEST_EVT     0x80000000

typedef  struct
{
	unsigned long counter;
	unsigned long distance;
	unsigned long calorie;
	unsigned long goal;
	unsigned char goal_per;
}PedometerInfo_Typedef;

extern PedometerInfo_Typedef pedometer, pedometer_last;

extern unsigned long taskStepTaskId;

extern void load_pedometer(unsigned long last_pedometer);

extern unsigned long taskStep( unsigned long task_id, unsigned long events );
extern void taskStepInit( unsigned long task_id );

extern void unit_update(void);

#endif
