#ifndef __SLEEP_ALGORITHM_H__
#define __SLEEP_ALGORITHM_H__

#include "ClockTick.h"

typedef struct 
{
	unsigned short action_cnt;
	unsigned short wave_cnt;
	unsigned long WaveAmplitudeSum;
  
	unsigned char prepareTimer;
	unsigned char prepareThreshold;
	unsigned char prepareTimerTheshold;
	unsigned char sleep_step;
	
	unsigned short sleep_Counter;
	unsigned char  sleep_type;
	
	unsigned long UTC_stime;   
	unsigned long UTC_lastStime;
	unsigned long UTC_nextLastStime;
	unsigned long UTC_etime;
	unsigned long UTC_lastEtime;
	unsigned long UTC_nextLastEtime;
	
	unsigned char   out_sleepProcess;   
	unsigned char   out_sleepEnable; 
	unsigned short  output_sleep_Counter[3];
	unsigned char   output_sleep_type[3];
	unsigned long  output_UTC_stime[3];
	unsigned long  output_UTC_etime[3];
	unsigned char   output_counter;
	
	unsigned short out_TotalSleepTime;
	unsigned short out_TotalLightTime;
	unsigned short out_TotalDeepTime;
	unsigned short out_TotalWakeTime;
}AutoSleep_t;

extern AutoSleep_t AutoSleep;

extern void sleep_algorithm(unsigned short action_cnt,\
	unsigned short wave_cnt,unsigned long WaveAmplitudeSum,\
		struct time_struct *ptimer);

#endif


