#ifndef __TASK_HR_H__
#define __TASK_HR_H__

#define TASK_HR_SAMPLE_EVT 				0x00000001
#define TASK_HR_STOP_EVT   				0x00000002
#define TASK_HR_START_EVT  				0x00000004
#define TASK_HR_DECT_DONE_HR_EVT    	0x00000008
#define TASK_HR_DECT_DONE_HR_UPLOAD_EVT 0x00000010
#define TASK_HR_FFT_ENABLE_EVT      	0x00000020

#define TASK_HR_LEAVE_EVT           	0x00000040
#define TASK_HR_NORMAL_EVT          	0x00000080
#define TASK_HR_DECT_AUTO_EVT       	0x00000100

#define TASK_HR_INIT_EVT                0x00000400
#define TASK_HR_DATA_PROCESSING_EVT     0x00000800

extern int to_close;
extern unsigned long need_mTicks;

extern unsigned char taskHrTaskId;
extern unsigned long taskHr( unsigned long task_id, unsigned long events );
extern void taskHrInit( unsigned long task_id );

#endif
