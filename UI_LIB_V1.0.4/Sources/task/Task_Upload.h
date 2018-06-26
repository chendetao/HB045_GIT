#ifndef __TASK_UPLOAD_H__
#define __TASK_UPLOAD_H__

extern int flash_memory_put( int identifier, const unsigned char *buf, int length);
extern int getFromNVFlash( int identifier, unsigned char *buf, int length );


struct Queue
{
	unsigned long front;
	unsigned long tail;
	unsigned long items;
	unsigned char lock;
};

extern struct Queue q;

#define QUEUE_ITEM_SIZE   16
#define QUEUE_LOCKED       -1
#define QUEUE_EMPTY        -2

extern int store_queue( int identifier, struct Queue *q, int length);
extern int load_queue( int identifier, struct Queue *q, unsigned char *buf, int length );


#define TASK_UPLOAD_PEDO_EVT  				0x00000001
#define TASK_UPLOAD_PEDO_1_EVT 				0x00000002
#define TASK_UPLOAD_PEDO_2_EVT 				0x00000004

#define TASK_UPLOAD_HR_EVT         			0x00000008
#define TASK_UPLOAD_HR_1_EVT       			0x00000010
#define TASK_UPLOAD_HR_2_EVT       			0x00000020

#define TASK_UPLOAD_SL_EVT      			0x00000040
#define TASK_UPLOAD_SL_1_EVT    			0x00000080
#define TASK_UPLOAD_SL_2_EVT    			0x00000100

#define TASK_UPLOAD_B_PRESSURE_EVT  		0x00000200
#define TASK_UPLOAD_B_PRESSURE_1_EVT		0x00000400
#define TASK_UPLOAD_B_PRESSURE_2_EVT		0x00000800

#define TASK_UPLOAD_SPO2_EVT        		0x00001000
#define TASK_UPLOAD_SP02_1_EVT          	0x00002000
#define TASK_UPLOAD_SP02_2_EVT          	0x00004000

#define TASK_UPLOAD_SEG_PEDO_1_EVT          0x00008000
#define TASK_UPLOAD_SEG_PEDO_2_EVT          0x00010000
#define TASK_UPLOAD_STORE_SEG_PEDO_EVT      0x00020000

#define TASK_UPLOAD_STORE_PEDO_EVT      	0x00040000
#define TASK_UPLOAD_STORE_SL_EVT        	0x00080000
#define TASK_UPLOAD_STORE_HR_EVT        	0x00100000
#define TASK_UPLOAD_STORE_B_P_EVT       	0x00200000
#define TASK_UPLOAD_STORE_SPO2_EVT      	0x00400000
#define TASK_UPLOAD_RESET_QUEUE_EVT     	0x00800000

#define TASK_UPLOAD_HR_DONE_EVT   			0x01000000
#define TASK_UPLOAD_iBand_HR_DECT_EVT   	0x02000000
#define TASK_UPLOAD_iBand_B_PRUESSURE_EVT 	0x04000000
#define TASK_UPLOAD_iBand_SPO2_EVT          0x08000000
#define TASK_UPLOAD_iBand_SEG_PEDO_EVT      0x10000000

#define TASK_UPLOAD_STORE_RUNNING_EVT       0x20000000
#define TASK_UPLOAD_STORE_RUNNING_1_EVT     0x40000000
#define TASK_UPLOAD_STORE_RUNNING_2_EVT     0x80000000

extern unsigned long taskUploadTaskId;

extern unsigned long taskUpload( unsigned long task_id, unsigned long events );
extern void taskUploadInit( unsigned long task_id );

#endif
