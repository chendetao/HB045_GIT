#ifndef __TASK_SOTRE_H__
#define __TASK_SOTRE_H__

#define TASK_STORE_UPLOAD_RUNNING_EVT   0x00000100
#define TASK_STORE_SAVE_SWIMMING_EVT    0x00000200
#define TASK_STORE_SAVE_RUNNING_EVT     0x00000080
#define TASK_STORE_SAVE_BYCLE_EVT       0x00000040
#define TASK_STORE_SAVE_SEG_PEDO_EVT    0x00000020
#define TASK_STORE_SAVE_SPO2_EVT       	0x00000010
#define TASK_STORE_SAVE_B_PRESSURE_EVT 	0x00000008
#define TASK_STORE_SAVE_HR_EVT     		0x00000002
#define TASK_STORE_SAVE_PEDO_EVT       	0x00000001

extern unsigned long taskStoreTaskId;

extern unsigned long taskStore( unsigned long task_id, unsigned long events );
extern void taskStoreInit( unsigned long task_id );

#endif
