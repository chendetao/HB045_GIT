#ifndef __TASK_FINDER_H__
#define __TASK_FINDER_H__

#define FIND_HONE_TASK_START_EVT        	0x00000001
#define FIND_HONE_TASK_STOP_EVT         	0x00000002
#define FINDPHONE_TASK_FINDME_START_EVT 	0x00000004
#define FINDPHONE_TASK_FINDME_STOP_EVT  	0x00000008
#define FINDPHONE_TASK_FINDME_RESPONSE_EVT 	0x00000010

#define FIND_PHONE_STATE_STOP     0
#define FIND_PHONE_STATE_FINDING  1

typedef struct
{
    unsigned char state:4;
    unsigned char isFound:2;
    unsigned char phoneFindMe:1;
	unsigned char cancelFindMe:1;
}Finder;

extern Finder finder;

extern unsigned long taskFinderTaskId;

extern unsigned long taskFinder( unsigned long task_id, unsigned long events );
extern void taskFinderInit( unsigned long task_id );


#endif
