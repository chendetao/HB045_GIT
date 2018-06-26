#include "sleep_algorithm.h"
#include "ClockTick.h"

AutoSleep_t  AutoSleep = {
	.action_cnt = 0,
	.wave_cnt   = 0,
	.out_sleepProcess = 0,
	.out_sleepEnable  = 0,
	.sleep_step   = 0,
	.prepareThreshold = 10,
	.prepareTimerTheshold = 20,
};

void SleepdataInit(struct time_struct *ptimer);
void Put_the_desktop_process(void);
void SaveSleepdata(struct time_struct *ptimer);

void sleep_algorithm(unsigned short action_cnt,unsigned short wave_cnt,
	unsigned long WaveAmplitudeSum,struct time_struct *ptimer)
{
	static unsigned char temp_cnt     = 0;
    static unsigned char temp_cnt1    = 0;
    static unsigned char g_start_step = 0;
	
	 /* 1.早上8点到中午12点，不进行入睡检测*/
	if((ptimer->hour>=8) && (ptimer->hour<12))
	{
	  AutoSleep.out_sleepProcess = 0;  /* 非入睡检测阶段*/
	  /*如果在这之前已经进入预备状态，强制进入活动状态，并清除进入预备状态时间*/
	  if(AutoSleep.sleep_step==0x1)
	  {
		AutoSleep.sleep_step   = 0x0;
	  }
	}

	/* 2.中午12点到14点，正常进行入睡检测*/
	else if((ptimer->hour>=12) && (ptimer->hour<14)) 
	{
	  AutoSleep.out_sleepProcess = 0;  /* 入睡检测阶段*/
	  AutoSleep.prepareThreshold = 10;
	}

	/* 3.下午14点到20点，不进行入睡检测*/
	else if((ptimer->hour>=14) && (ptimer->hour<20)) 
	{
	  AutoSleep.out_sleepProcess = 0;  /* 非入睡检测阶段*/
	  /*如果在这之前已经进入预备状态，强制进入活动状态*/
	  if(AutoSleep.sleep_step==0x1)
	  {
		AutoSleep.sleep_step   = 0x0;
	  }
	}

	/* 4.晚上8点到晚上23点，进行入睡检测，但入睡条件改变*/
	else if((ptimer->hour>=20) && (ptimer->hour<23)) 
	{
	  AutoSleep.out_sleepProcess     = 1;  /* 入睡检测阶段*/
	  AutoSleep.prepareThreshold     = 9;
	  AutoSleep.prepareTimerTheshold = 25;
	}
	/* 5.晚上23点到早上8点，正常进行入睡检测*/
	else if((ptimer->hour>=23) || (ptimer->hour<8))
	{
	  AutoSleep.out_sleepProcess     = 1;  /* 入睡检测阶段*/
	  AutoSleep.prepareThreshold     = 12;
	  AutoSleep.prepareTimerTheshold = 15;
	}
	
	AutoSleep.wave_cnt         = wave_cnt;
	AutoSleep.WaveAmplitudeSum = WaveAmplitudeSum;
	AutoSleep.action_cnt       = action_cnt;
	
	
	/* 在入睡检测阶段或非入睡检测阶段但在睡眠状态 */
    if ( (AutoSleep.out_sleepProcess == 0x1) || (AutoSleep.out_sleepEnable == 0x1))
    {
		switch(AutoSleep.sleep_step)  /* 状态 */
		{
			case 0: 
					if(/*(AutoSleep.action_cnt>1) &&*/ (AutoSleep.wave_cnt<8) )
					{
						/*几分钟较少的动作,由活动状态进入预备状态*/
						SleepdataInit(ptimer);
						AutoSleep.sleep_Counter = 0;

						AutoSleep.sleep_step   = 1;
						AutoSleep.prepareTimer = 0;
						temp_cnt  = 0;
						temp_cnt1 = 0;
					}
					break;
			/* 在预备状态中一旦检测到大幅度动作，重新进入活动状态 */
			case 1: if(AutoSleep.wave_cnt>AutoSleep.prepareThreshold)
					{
						AutoSleep.prepareTimer = 0;
						AutoSleep.out_sleepEnable = 0;
						AutoSleep.sleep_step = 0;
					}
					/* 预备状态结束，进入浅睡状态 */
					else
					{
						if((AutoSleep.wave_cnt==0) || (AutoSleep.action_cnt<=15)) { temp_cnt++; }
						else                                                      { temp_cnt = 0; }
						
						
						AutoSleep.prepareTimer++;
						if(AutoSleep.prepareTimer>AutoSleep.prepareTimerTheshold)
						{
							if(temp_cnt>=(AutoSleep.prepareTimerTheshold - 5))
							{
								AutoSleep.sleep_step  = 3; 
							}
							else           
							{ 
								AutoSleep.sleep_step  = 2; 
							}
							
							AutoSleep.sleep_Counter    = AutoSleep.prepareTimer + 1;
							g_start_step               = 0x1; /*不需要重新计算开始时间*/
							
							
							AutoSleep.out_sleepEnable  = 1;
							AutoSleep.prepareTimer = 0;
							temp_cnt               = 0;
							temp_cnt1              = 0;
						}
					}
					break;

			case 2: if(g_start_step==0x0)
					{
						g_start_step = 0x1;
						SleepdataInit(ptimer);
					}

					AutoSleep.sleep_Counter++;
					if(AutoSleep.action_cnt==0)
					{
						temp_cnt++;
						if(temp_cnt>=7)
						{
							{   /* 浅睡状态一直没有动作，进入深睡状态*/
								temp_cnt1            = 0;
								temp_cnt             = 0;
								AutoSleep.sleep_step = 3;
							}

							/* 如果浅睡时间过短，直接把这段时间补偿到深睡状态 */
							if(AutoSleep.sleep_Counter<=5)
							{
								g_start_step = 0x1;  /*不需要重新计算开始时间*/
							}
							else
							{
								/* 保存这段浅睡时间*/
								AutoSleep.sleep_type = 0x02;
								SaveSleepdata(ptimer);
								AutoSleep.sleep_Counter = 0x0;
								g_start_step      = 0x0; 
							}

						}
					}
					else 
					{
						temp_cnt = 0;
					}

					/* 浅睡状态，这个时候如果检测到频繁的动作，认为睡眠结束*/
					if(AutoSleep.wave_cnt>20)
					{
						temp_cnt1++;
						if(temp_cnt1>=3)
						{
							if(AutoSleep.sleep_Counter>=5)
							{
								AutoSleep.sleep_type = 0x02;
								SaveSleepdata(ptimer);
							}
							AutoSleep.sleep_Counter = 0x0;
							g_start_step            = 0x0; 

							AutoSleep.out_sleepEnable = 0;
							AutoSleep.sleep_step = 0;
						}
					}
					else
					{
						temp_cnt1 = 0;
					}
					break;

			case 3: if(g_start_step==0x0)
					{
						g_start_step = 0x1;
						SleepdataInit(ptimer);
					}
					AutoSleep.sleep_Counter++;
					if(AutoSleep.wave_cnt>=4)
					{
						{
							/* 深睡状态有较多动作，进入浅睡状态*/
							AutoSleep.sleep_step = 2;
							temp_cnt = 0;
							temp_cnt1 = 0;
						}

						/* 如果深睡时间过短，直接把这段时间补偿到浅睡状态 */
						if(AutoSleep.sleep_Counter<=5)
						{
							g_start_step = 0x1;  /*不需要重新计算开始时间*/
						}
						else
						{
							/* 保存深睡时间*/
							AutoSleep.sleep_type    = 0x01;
							SaveSleepdata( ptimer );
							AutoSleep.sleep_Counter = 0x0;
							g_start_step            = 0x0; 
						}
					}
					break;

			case 0x4:if(AutoSleep.wave_cnt>0)
					{
						AutoSleep.sleep_step = 0x0;
						AutoSleep.out_sleepEnable = 0x0;  //bug
					}
					break;
		}
		/* 未识别为放置状态，则开启放置桌面识别进程 */
		if(AutoSleep.sleep_step!=0x4)
		{
			Put_the_desktop_process();
		}
	}
}




void SleepdataInit(struct time_struct *ptimer)
{
    /*几分钟较少的动作,由活动状态进入预备状态*/
	AutoSleep.UTC_stime = Convert_UTC_time(ptimer);
}


void Put_the_desktop_process(void)
{
	static unsigned char temp_cnt2    = 0;

/*
	1.30分钟内无波形，且之前无睡眠，认为此时防止在桌面
	2.30分钟内无波形，且之前有睡眠，继续观察
	3.45分钟无波形，此时无论之前有无睡眠，认为此时防止在桌面
*/
	
	if((AutoSleep.wave_cnt==0x0) || (AutoSleep.action_cnt<=15))
	{

	  temp_cnt2++;
	   
	  /* 30分钟内无波形，且之前无睡眠，认为此时防止在桌面*/
	  if((temp_cnt2>=30) && (AutoSleep.UTC_stime-AutoSleep.UTC_lastEtime>=180))  
	  {
		 temp_cnt2 = 0;
		/*睡了一段时间放到桌面上*/
		AutoSleep.out_sleepEnable = 0;
		AutoSleep.sleep_step = 0x4;
	  }
	  /* 45分钟无波形，此时无论之前有无睡眠，认为此时防止在桌面*/
	  else if(temp_cnt2>=45)
	  {
		  temp_cnt2 = 0;
		  /*睡了一段时间放到桌面上*/
		  AutoSleep.out_sleepEnable = 0;
		  AutoSleep.sleep_step = 0x4;
	  }
	}
	else 
	{
	  temp_cnt2 = 0;
	}
}


void SaveSleepdata(struct time_struct *ptimer)
{
	static unsigned char have_sleep_data = 0;

	AutoSleep.UTC_etime = Convert_UTC_time(ptimer);
		
	/* 
		1.距离上一段数据时间很近(当前的开始 - 上一段的结束)
		  1>如果有数据数据未存，把上段未存数据补上
		  2>直接存储本段数据 
		  
		2.距离上一段数据时间较久
		  1>如果有数据未存，时间隔太远了,直接清除临时缓冲区数据
		  2>把本段写进临时缓冲区
	*/	
	
		
	 /* 距离上一段时间很近*/
	if((AutoSleep.UTC_stime-AutoSleep.UTC_lastEtime)<=180)
	{
		/* 如果有数据未存，把上段未存数据补上*/
		if(have_sleep_data==0x1)
		{
			unsigned short last_out_TotalSleepTime = 0;
			
			last_out_TotalSleepTime = AutoSleep.out_TotalSleepTime;
			
			/* 1.1 睡眠总数统计增加 */
			AutoSleep.out_TotalSleepTime += AutoSleep.output_sleep_Counter[AutoSleep.output_counter];
			if(AutoSleep.output_sleep_type[AutoSleep.output_counter]==0x1)
			{
				AutoSleep.out_TotalDeepTime += AutoSleep.output_sleep_Counter[AutoSleep.output_counter];
			}
			else 
			{
				AutoSleep.out_TotalLightTime += AutoSleep.output_sleep_Counter[AutoSleep.output_counter];
			}
			
			/* 1.2 生成单段数据 */
			AutoSleep.output_counter++;
			
			/* 2.增加醒来数据 */
			if(last_out_TotalSleepTime!=0x0)
			{
				unsigned long wake_counter = 0;
				
				/* 2.1 生成单段数据 */
				AutoSleep.output_UTC_stime[AutoSleep.output_counter]     = AutoSleep.UTC_nextLastEtime + 60;
				AutoSleep.output_UTC_etime[AutoSleep.output_counter]     = AutoSleep.UTC_lastStime - 60;
				
				wake_counter = AutoSleep.output_UTC_etime[AutoSleep.output_counter] - AutoSleep.output_UTC_stime[AutoSleep.output_counter];
				wake_counter /= 60;
				AutoSleep.output_sleep_Counter[AutoSleep.output_counter] = wake_counter;
				AutoSleep.output_sleep_type[AutoSleep.output_counter]    = 0x03;
				AutoSleep.output_counter++;
				
				/* 2.2 睡眠总数统计增加*/
				AutoSleep.out_TotalWakeTime += wake_counter;
				
				/* 2017-6-30 把醒来时间放到前面 */
				{
					unsigned short  output_sleep_Counter;
					unsigned char   output_sleep_type;
					unsigned long  output_UTC_stime;
					unsigned long  output_UTC_etime;
					
					output_UTC_stime     = AutoSleep.output_UTC_stime[0];
					output_UTC_etime     = AutoSleep.output_UTC_etime[0];
					output_sleep_type    = AutoSleep.output_sleep_type[0];
					output_sleep_Counter = AutoSleep.output_sleep_Counter[0];
					 
					AutoSleep.output_UTC_stime[0]     = AutoSleep.output_UTC_stime[1];
					AutoSleep.output_UTC_etime[0]     = AutoSleep.output_UTC_etime[1];
					AutoSleep.output_sleep_type[0]    = AutoSleep.output_sleep_type[1];
					AutoSleep.output_sleep_Counter[0] = AutoSleep.output_sleep_Counter[1];
					
					AutoSleep.output_UTC_stime[1]     = output_UTC_stime;
					AutoSleep.output_UTC_etime[1]     = output_UTC_etime;
					AutoSleep.output_sleep_type[1]    = output_sleep_type;
					AutoSleep.output_sleep_Counter[1] = output_sleep_Counter;
					
				}
			}
			
			have_sleep_data = 0x0;
		}
		
		/* 3.1 生成单段数据 */
		AutoSleep.output_UTC_stime[AutoSleep.output_counter]     = AutoSleep.UTC_stime;
		AutoSleep.output_UTC_etime[AutoSleep.output_counter]     = AutoSleep.UTC_etime;
		AutoSleep.output_sleep_Counter[AutoSleep.output_counter] = AutoSleep.sleep_Counter;
		AutoSleep.output_sleep_type[AutoSleep.output_counter]    = AutoSleep.sleep_type;
		
		/* 3.2 睡眠总数统计增加 */
		AutoSleep.out_TotalSleepTime += AutoSleep.output_sleep_Counter[AutoSleep.output_counter];
		if(AutoSleep.output_sleep_type[AutoSleep.output_counter]==0x1)
		{
			AutoSleep.out_TotalDeepTime += AutoSleep.output_sleep_Counter[AutoSleep.output_counter];
		}
		else 
		{
			AutoSleep.out_TotalLightTime += AutoSleep.output_sleep_Counter[AutoSleep.output_counter];
		}
		
		AutoSleep.output_counter++;
		
	}

	/* 距离上一段时间较久，先不存，如果有接下来的一段睡眠，才把该段存储*/
	else 
	{
		/* 如果有数据未存，时间隔太远了，直接清除数据 */
		if(have_sleep_data==0x01)
		{
		  have_sleep_data = 0x0;
		}
		/* 生成单段数据 */
		AutoSleep.output_UTC_stime[AutoSleep.output_counter]     = AutoSleep.UTC_stime;
		AutoSleep.output_UTC_etime[AutoSleep.output_counter]     = AutoSleep.UTC_etime;
		AutoSleep.output_sleep_Counter[AutoSleep.output_counter] = AutoSleep.sleep_Counter;
		AutoSleep.output_sleep_type[AutoSleep.output_counter]    = AutoSleep.sleep_type;
		
		have_sleep_data = 1; /* 有睡眠数据未提交 */
	}
	
	/* 记录最近三次的UTC时间 */
	AutoSleep.UTC_nextLastStime = AutoSleep.UTC_lastStime;
	AutoSleep.UTC_nextLastEtime = AutoSleep.UTC_lastEtime;
	AutoSleep.UTC_lastStime = AutoSleep.UTC_stime;
	AutoSleep.UTC_lastEtime = AutoSleep.UTC_etime;
}

#include "OSAL.h"  
#include <string.h>
#include "Task_Flash.h"

const unsigned char rand_start_hh[] = {0x23,0x00,0x00,0x01,0x01,0x02,0x02};
const unsigned char rand_start_mi[] = {0x32,0x00,0x35,0x06,0x44,0x24,0x56};

const unsigned char  rand_stop_hh[] = {0x23,0x00,0x01,0x01,0x02,0x02,0x03};
const unsigned char  rand_stop_mi[] = {0x59,0x34,0x05,0x43,0x23,0x55,0x10};	

const unsigned char rand_timer[] =    {  27,  29,  30,  37,  39,  31,  26};

extern void put_sleep(unsigned char *buf);

void generate_test_data(void)
{
	unsigned char buffer[16];
	
	int yy = TimerYY();
	int mm = TimerMM();
	int dd = TimerDD() - 1;
	
	AutoSleep.out_TotalDeepTime  = 0;
	AutoSleep.out_TotalLightTime = 0;
	AutoSleep.out_TotalSleepTime = 0;
	
	for ( int i = 0; i < SIZEOF(rand_start_hh); i++ )
	{
		memset ( buffer, 0, 16 );
		
		buffer[0] = DECToBCD(yy);
		buffer[1] = DECToBCD(mm);
		buffer[2] = DECToBCD(dd);
		buffer[3] = rand_start_hh[i];
		buffer[4] = rand_start_mi[i];
	
		buffer[5] = DECToBCD(yy);
		buffer[6] = DECToBCD(mm);
		buffer[7] = DECToBCD(dd);
		buffer[8] = rand_stop_hh[i];
		buffer[9] = rand_stop_mi[i];
	
		buffer[10] = rand_timer[i]>>8;
		buffer[11] = rand_timer[i]&0xFF;	
		
		if (  i & 0x1 )
		{
			buffer[12] = 0x1;
			AutoSleep.out_TotalDeepTime += rand_timer[i];
		}else{
			buffer[12] = 0x2;
            AutoSleep.out_TotalLightTime += rand_timer[i];
		}	

		AutoSleep.out_TotalSleepTime += rand_timer[i];
	
		// put into queue for save
		
		put_sleep(buffer);	
		
		if ( i == 0x0 )
		{
			dd += 1;
		}
	}		
}


