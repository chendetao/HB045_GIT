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
	
	 /* 1.����8�㵽����12�㣬��������˯���*/
	if((ptimer->hour>=8) && (ptimer->hour<12))
	{
	  AutoSleep.out_sleepProcess = 0;  /* ����˯���׶�*/
	  /*�������֮ǰ�Ѿ�����Ԥ��״̬��ǿ�ƽ���״̬�����������Ԥ��״̬ʱ��*/
	  if(AutoSleep.sleep_step==0x1)
	  {
		AutoSleep.sleep_step   = 0x0;
	  }
	}

	/* 2.����12�㵽14�㣬����������˯���*/
	else if((ptimer->hour>=12) && (ptimer->hour<14)) 
	{
	  AutoSleep.out_sleepProcess = 0;  /* ��˯���׶�*/
	  AutoSleep.prepareThreshold = 10;
	}

	/* 3.����14�㵽20�㣬��������˯���*/
	else if((ptimer->hour>=14) && (ptimer->hour<20)) 
	{
	  AutoSleep.out_sleepProcess = 0;  /* ����˯���׶�*/
	  /*�������֮ǰ�Ѿ�����Ԥ��״̬��ǿ�ƽ���״̬*/
	  if(AutoSleep.sleep_step==0x1)
	  {
		AutoSleep.sleep_step   = 0x0;
	  }
	}

	/* 4.����8�㵽����23�㣬������˯��⣬����˯�����ı�*/
	else if((ptimer->hour>=20) && (ptimer->hour<23)) 
	{
	  AutoSleep.out_sleepProcess     = 1;  /* ��˯���׶�*/
	  AutoSleep.prepareThreshold     = 9;
	  AutoSleep.prepareTimerTheshold = 25;
	}
	/* 5.����23�㵽����8�㣬����������˯���*/
	else if((ptimer->hour>=23) || (ptimer->hour<8))
	{
	  AutoSleep.out_sleepProcess     = 1;  /* ��˯���׶�*/
	  AutoSleep.prepareThreshold     = 12;
	  AutoSleep.prepareTimerTheshold = 15;
	}
	
	AutoSleep.wave_cnt         = wave_cnt;
	AutoSleep.WaveAmplitudeSum = WaveAmplitudeSum;
	AutoSleep.action_cnt       = action_cnt;
	
	
	/* ����˯���׶λ����˯���׶ε���˯��״̬ */
    if ( (AutoSleep.out_sleepProcess == 0x1) || (AutoSleep.out_sleepEnable == 0x1))
    {
		switch(AutoSleep.sleep_step)  /* ״̬ */
		{
			case 0: 
					if(/*(AutoSleep.action_cnt>1) &&*/ (AutoSleep.wave_cnt<8) )
					{
						/*�����ӽ��ٵĶ���,�ɻ״̬����Ԥ��״̬*/
						SleepdataInit(ptimer);
						AutoSleep.sleep_Counter = 0;

						AutoSleep.sleep_step   = 1;
						AutoSleep.prepareTimer = 0;
						temp_cnt  = 0;
						temp_cnt1 = 0;
					}
					break;
			/* ��Ԥ��״̬��һ����⵽����ȶ��������½���״̬ */
			case 1: if(AutoSleep.wave_cnt>AutoSleep.prepareThreshold)
					{
						AutoSleep.prepareTimer = 0;
						AutoSleep.out_sleepEnable = 0;
						AutoSleep.sleep_step = 0;
					}
					/* Ԥ��״̬����������ǳ˯״̬ */
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
							g_start_step               = 0x1; /*����Ҫ���¼��㿪ʼʱ��*/
							
							
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
							{   /* ǳ˯״̬һֱû�ж�����������˯״̬*/
								temp_cnt1            = 0;
								temp_cnt             = 0;
								AutoSleep.sleep_step = 3;
							}

							/* ���ǳ˯ʱ����̣�ֱ�Ӱ����ʱ�䲹������˯״̬ */
							if(AutoSleep.sleep_Counter<=5)
							{
								g_start_step = 0x1;  /*����Ҫ���¼��㿪ʼʱ��*/
							}
							else
							{
								/* �������ǳ˯ʱ��*/
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

					/* ǳ˯״̬�����ʱ�������⵽Ƶ���Ķ�������Ϊ˯�߽���*/
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
							/* ��˯״̬�н϶ද��������ǳ˯״̬*/
							AutoSleep.sleep_step = 2;
							temp_cnt = 0;
							temp_cnt1 = 0;
						}

						/* �����˯ʱ����̣�ֱ�Ӱ����ʱ�䲹����ǳ˯״̬ */
						if(AutoSleep.sleep_Counter<=5)
						{
							g_start_step = 0x1;  /*����Ҫ���¼��㿪ʼʱ��*/
						}
						else
						{
							/* ������˯ʱ��*/
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
		/* δʶ��Ϊ����״̬��������������ʶ����� */
		if(AutoSleep.sleep_step!=0x4)
		{
			Put_the_desktop_process();
		}
	}
}




void SleepdataInit(struct time_struct *ptimer)
{
    /*�����ӽ��ٵĶ���,�ɻ״̬����Ԥ��״̬*/
	AutoSleep.UTC_stime = Convert_UTC_time(ptimer);
}


void Put_the_desktop_process(void)
{
	static unsigned char temp_cnt2    = 0;

/*
	1.30�������޲��Σ���֮ǰ��˯�ߣ���Ϊ��ʱ��ֹ������
	2.30�������޲��Σ���֮ǰ��˯�ߣ������۲�
	3.45�����޲��Σ���ʱ����֮ǰ����˯�ߣ���Ϊ��ʱ��ֹ������
*/
	
	if((AutoSleep.wave_cnt==0x0) || (AutoSleep.action_cnt<=15))
	{

	  temp_cnt2++;
	   
	  /* 30�������޲��Σ���֮ǰ��˯�ߣ���Ϊ��ʱ��ֹ������*/
	  if((temp_cnt2>=30) && (AutoSleep.UTC_stime-AutoSleep.UTC_lastEtime>=180))  
	  {
		 temp_cnt2 = 0;
		/*˯��һ��ʱ��ŵ�������*/
		AutoSleep.out_sleepEnable = 0;
		AutoSleep.sleep_step = 0x4;
	  }
	  /* 45�����޲��Σ���ʱ����֮ǰ����˯�ߣ���Ϊ��ʱ��ֹ������*/
	  else if(temp_cnt2>=45)
	  {
		  temp_cnt2 = 0;
		  /*˯��һ��ʱ��ŵ�������*/
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
		1.������һ������ʱ��ܽ�(��ǰ�Ŀ�ʼ - ��һ�εĽ���)
		  1>�������������δ�棬���϶�δ�����ݲ���
		  2>ֱ�Ӵ洢�������� 
		  
		2.������һ������ʱ��Ͼ�
		  1>���������δ�棬ʱ���̫Զ��,ֱ�������ʱ����������
		  2>�ѱ���д����ʱ������
	*/	
	
		
	 /* ������һ��ʱ��ܽ�*/
	if((AutoSleep.UTC_stime-AutoSleep.UTC_lastEtime)<=180)
	{
		/* ���������δ�棬���϶�δ�����ݲ���*/
		if(have_sleep_data==0x1)
		{
			unsigned short last_out_TotalSleepTime = 0;
			
			last_out_TotalSleepTime = AutoSleep.out_TotalSleepTime;
			
			/* 1.1 ˯������ͳ������ */
			AutoSleep.out_TotalSleepTime += AutoSleep.output_sleep_Counter[AutoSleep.output_counter];
			if(AutoSleep.output_sleep_type[AutoSleep.output_counter]==0x1)
			{
				AutoSleep.out_TotalDeepTime += AutoSleep.output_sleep_Counter[AutoSleep.output_counter];
			}
			else 
			{
				AutoSleep.out_TotalLightTime += AutoSleep.output_sleep_Counter[AutoSleep.output_counter];
			}
			
			/* 1.2 ���ɵ������� */
			AutoSleep.output_counter++;
			
			/* 2.������������ */
			if(last_out_TotalSleepTime!=0x0)
			{
				unsigned long wake_counter = 0;
				
				/* 2.1 ���ɵ������� */
				AutoSleep.output_UTC_stime[AutoSleep.output_counter]     = AutoSleep.UTC_nextLastEtime + 60;
				AutoSleep.output_UTC_etime[AutoSleep.output_counter]     = AutoSleep.UTC_lastStime - 60;
				
				wake_counter = AutoSleep.output_UTC_etime[AutoSleep.output_counter] - AutoSleep.output_UTC_stime[AutoSleep.output_counter];
				wake_counter /= 60;
				AutoSleep.output_sleep_Counter[AutoSleep.output_counter] = wake_counter;
				AutoSleep.output_sleep_type[AutoSleep.output_counter]    = 0x03;
				AutoSleep.output_counter++;
				
				/* 2.2 ˯������ͳ������*/
				AutoSleep.out_TotalWakeTime += wake_counter;
				
				/* 2017-6-30 ������ʱ��ŵ�ǰ�� */
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
		
		/* 3.1 ���ɵ������� */
		AutoSleep.output_UTC_stime[AutoSleep.output_counter]     = AutoSleep.UTC_stime;
		AutoSleep.output_UTC_etime[AutoSleep.output_counter]     = AutoSleep.UTC_etime;
		AutoSleep.output_sleep_Counter[AutoSleep.output_counter] = AutoSleep.sleep_Counter;
		AutoSleep.output_sleep_type[AutoSleep.output_counter]    = AutoSleep.sleep_type;
		
		/* 3.2 ˯������ͳ������ */
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

	/* ������һ��ʱ��Ͼã��Ȳ��棬����н�������һ��˯�ߣ��ŰѸöδ洢*/
	else 
	{
		/* ���������δ�棬ʱ���̫Զ�ˣ�ֱ��������� */
		if(have_sleep_data==0x01)
		{
		  have_sleep_data = 0x0;
		}
		/* ���ɵ������� */
		AutoSleep.output_UTC_stime[AutoSleep.output_counter]     = AutoSleep.UTC_stime;
		AutoSleep.output_UTC_etime[AutoSleep.output_counter]     = AutoSleep.UTC_etime;
		AutoSleep.output_sleep_Counter[AutoSleep.output_counter] = AutoSleep.sleep_Counter;
		AutoSleep.output_sleep_type[AutoSleep.output_counter]    = AutoSleep.sleep_type;
		
		have_sleep_data = 1; /* ��˯������δ�ύ */
	}
	
	/* ��¼������ε�UTCʱ�� */
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


