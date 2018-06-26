#include "OSAL_Timer.h"
#include "OSAL.h"
#include "Task_GUI.h"
#include "Task_Step.h"
#include "Task_Battery.h"
#include "Task_Upload.h"
#include "Task_Flash.h"
#include "Task_Store.h"
#include "Task_Ble.h"

#include "HeartComp.h"
#include "ClockTick.h"

struct time_struct current_timer = { DEFAULT_HOUR,DEFAULT_MINUTE,DEFAULT_SECOND,
                             DEFAULT_YEAR,DEFAULT_MONTH,DEFAULT_DAY,
                             DEFAULT_WEEK }; /* BCD 2016.9.20 DEX 08:00:00 */

const unsigned int mtable[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

struct time_struct last_timer;

void system_clock_click(unsigned long seconds )
{    
    current_timer.second += seconds;
	
	osal_set_event ( taskBleTaskId, TASK_BLE_WDT_FEED_EVT );	
    /**
     * 每隔UPDATE_RATE秒, GSensor传入XYZ到动态心率算法.
     */
	#if 1
    if ( !(TimerSE() % UPDATE_RATE))
    {
		osal_set_event( taskStepTaskId, TASK_STEP_HR_COMPENT_EVT);
    }
	#endif
	
	#if 1
	if ( !( TimerSE() % BATTERY_RATE) )
	{
		osal_set_event ( taskBatteryTaskId, TASK_BATTERY_DECT_EVT );
	}
	#endif	
    
    if(current_timer.second < 60){
        return;
    }
            
    current_timer.minute++; current_timer.second = 0;

    /* 一分钟到啦，通知关注本事件的任务 */
	
	#if 1
	// 电池电量检测事件
	osal_set_event ( taskBatteryTaskId, TASK_MINITE_TIMEOUT_EVT );
	#endif
	
	#if 1
	// UI界面超时(用于检测闹钟)
	osal_set_event ( taskGUITaskId, TASK_UI_MINUTE_TIMEOUT_EVT );
	#endif
	
	#if 1
	// 睡眠监测
	osal_set_event ( taskStepTaskId, STEP_TASK_SLEEP_DETECT_TICK_EVT );
	#endif
	
	#if 1
	// 更新当前配置信息到Flash
	if ( !(current_timer.minute % 15) )
	{
		osal_set_event( taskFlashTaskId, TASK_FLASH_UPDATE_CONFIG_EVT);
	}
	#endif
	
    if(current_timer.minute < 60 ){
        return;
    }
             
    current_timer.hour++; current_timer.minute = 0;
	
	// 分段计步保存/上报
	osal_start_timerEx ( taskStoreTaskId, TASK_STORE_SAVE_SEG_PEDO_EVT,2500);
	osal_start_timerEx ( taskUploadTaskId, TASK_UPLOAD_iBand_SEG_PEDO_EVT, 5000);	
	
	/** 晚8点清除睡眠界面数据 */
	if ( current_timer.hour == 20 )
	{
		osal_set_event( taskStepTaskId, STEP_TASK_CLEAR_SLEEP_EVT );
	}
        
    if(current_timer.hour < 24){
        return;
    }
	
	/** 一天到了 */
	#if 1
	osal_start_timerEx ( taskStoreTaskId, TASK_STORE_SAVE_PEDO_EVT, 10000);
	last_timer.year   = TimerYY();
	last_timer.month  = TimerMM();
	last_timer.day    = TimerDD();
	last_timer.hour   = TimerHH();
	last_timer.minute = TimerMI();
	last_timer.second = TimerSE();
	#endif
    
    current_timer.hour = 0; current_timer.day++; 
    current_timer.week++; current_timer.week %= 7;
      
        
    int r = 0;
    if((((current_timer.year % 4) == 0) && ((current_timer.year % 100) != 0)) 
       || ((current_timer.year % 400) == 0)){
        r = 1;
    }
    
    if ( current_timer.month != 2 )
    {
        r = 0;
    }

    if(current_timer.day <= ( (r) ?
		mtable[current_timer.month-1]+1 : 
		mtable[current_timer.month-1])){
        return;
    }
        
    current_timer.day = 1; current_timer.month++;
    if(current_timer.month < 13){
        return;
    }
       
    current_timer.month = 1; current_timer.year++;
}

void setSystemTimer(const struct time_struct *new_timer)
{
    current_timer.hour = new_timer->hour;
    current_timer.minute = new_timer->minute;
    current_timer.second = new_timer->second;
    
    current_timer.year = new_timer->year;
    current_timer.month = new_timer->month;
    current_timer.day = new_timer->day;
}

int SetTimer( unsigned char hour, unsigned char minute, unsigned char second)
{
    if( ( hour > 0x23) || (minute > 0x59) || (second > 0x59 ))
    {
        return -1;
    }
    
    /* 从 BCD 转换为十进制格式，方便运算 */
    current_timer.hour = ((hour>>4) * 10) + (hour & 0xF); 
    current_timer.minute = ((minute>>4) * 10 ) + (minute & 0xF);
    current_timer.second = ((second>>4) * 10 ) + (second & 0xF);
    
    return 0;
}

int SetDate( unsigned char year, unsigned char month, unsigned char day,
                      unsigned char week)
{
    if ( (year > 0x99) || (month > 0x12) || (day > 0x31) || (week > 0x6) )
    {
        return -1;
    }
    
    /* 从 BCD 转换为十进制格式，方便运算 */
    current_timer.year = ((year>>4) * 10) + (year & 0xF); ;
    current_timer.month = ((month>>4) * 10) + (month & 0xF); ;
    current_timer.day = ((day>>4) * 10) + (day & 0xF);
    current_timer.week = week;

    return 0;
}

void load_date_time(const struct time_struct *new_timer)
{
	setSystemTimer(new_timer);
}

#include "time.h"

unsigned long getUTC(void)
{	
	unsigned long utc;
	struct tm time;
	
	time.tm_hour = TimerHH();
	time.tm_min = TimerMI();
	time.tm_sec = TimerSE();
	time.tm_mday = TimerDD();
	time.tm_mon = TimerMM()-1;
	time.tm_year = TimerYY()+2000 - 1900;
	
	utc = mktime(&time);
	
	return utc;
}

unsigned long Convert_UTC_time(struct time_struct *t)
{
	struct tm time;
	unsigned long utc;
	
	time.tm_hour = t->hour;
	time.tm_min = t->minute;
	time.tm_sec = t->second;
	time.tm_mday = t->day;
	time.tm_mon = t->month-1;
	
	time.tm_year = t->year +2000-1900;
	utc = mktime(&time);
	
	return utc;
}


/****************************************************************************
* 名    称：u8 Get_Week(u16 Year,u16 Month,u16 Date)
* 功    能：用蔡勒（Zeller）公式计算星期几	

	w=y+[y/4]+[c/4]-2c+[26（m+1）/10]+d-1

	然后w对7取余。
	公式中的符号含义如下，w：星期；c：世纪；y：年（两位数）；m：月（m大于等于3，小
	于等于14，即在蔡勒公式中，某年的1、2月要看作上一年的13、14月来计算，比如2003年1月1
	日要看作2002年的13月1日来计算）；d：日；[ ]代表取整，即只要整数部分。

* 入口参数：Year:年; Month:月; Date:日。
* 出口参数：星期几
* 说    明：
* 调用方法：无
****************************************************************************/ 
unsigned char Get_Week(unsigned short Year,unsigned short Month,unsigned short Date)
{
	unsigned char W,Y,C,M,D;
	
	C = Year/100;
	Y = Year%100;
	M = Month;
	if(M<3){M+=12;Y--;}
	D = Date;
	W = (Y + Y/4 + C/4 - 2*C + 26*(M+1)/10 + D - 1)%7;
	return W;
}

const unsigned char Mon_Table[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};		//平年的月份日期表

/****************************************************************************
* 名    称：u8 Is_Leap_Year(u16 year)
* 功    能：判断是否是闰年
* 入口参数：year
* 出口参数：1:是闰年；0:非闰年
* 说    明：
* 调用方法：无 
****************************************************************************/ 
unsigned char Is_LeapYear(unsigned short year)
{
    if ( (year%4) == 0 ) //必须能被4整除
    { 
        if ( (year%100) == 0 ) 
        {
            if ( (year%400) == 0 )return 1;//如果以00结尾,还要能被400整除
            else return 0; 
        }
        else return 1; 
    }
    else return 0;
}

/****************************************************************************
* 名    称：void Cal_Time(u32 Count,struct T_STRUCT *Time)
* 功    能：根据32位Count计数值计算日期和时间
* 入口参数：Count
* 出口参数：T_STRUCT* Time
* 说    明：
* 调用方法：Cal_Time(Count,&Real_Time);
****************************************************************************/ 
void Cal_Time(unsigned long Count,struct time_struct *Time)
{
	unsigned long SecTemp=0,DateTemp=0,Temp=0;
	unsigned long year;
	
	DateTemp = Count/86400;		//天数
	if(DateTemp!=0)		//超过1天
	{
		//确定年
		Temp = 1970;
		while(DateTemp>=365)
		{
			if(Is_LeapYear(Temp))	//闰年?
			{
				if(DateTemp>=366) DateTemp -= 366;
				else 
				{
					break;
				}
			}
			else DateTemp -= 365;
			Temp++;
		}
		year = Temp;
		
		//确定月
		Temp = 1;
		while(DateTemp>=28)
		{
			if(Is_LeapYear(year)&&Temp==2)
			{
				if(DateTemp>=29)DateTemp -= 29;//闰年的二月天数
				else break; 
			}
			else
			{
				if(DateTemp>=Mon_Table[Temp]) DateTemp -= Mon_Table[Temp];
				else break;
			}
			Temp++;
		}
		Time->month = Temp;		
		Time->day   = DateTemp+1;		
	}
	else
	{
		year = 1970;
		Time->month = 1;
		Time->day = 1;
	}
	SecTemp       = Count%86400;
	Time->hour    = SecTemp/3600;
	Time->minute  = SecTemp%3600/60;
	Time->second  = SecTemp%60;
	Time->week    = Get_Week(year,Time->month,Time->day);
	Time->year    = year - 2000;
}
