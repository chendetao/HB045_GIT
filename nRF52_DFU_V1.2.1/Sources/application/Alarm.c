
#include "ClockTick.h"
#include "Alarm.h"

#define AEN(x) ((unsigned long)(1)<<(x*2))   /* 使能闹钟位 */
#define DIS(x) ((unsigned long)(2)<<(x*2))   /* 取消闹钟位 */
#define NEN(x) ((unsigned short)(1)<<(x))     /* 附件条件：触摸取消相关 */
#define DEN(x) (~((unsigned short)(1)<<(x)))     /* 附件条件：触摸取消相关 */

Alarm alarm =
{
    .al   = {
                {0, 0},{0,0},{0, 0},
                {0, 0},{0,0},{0, 0},
                {0, 0},{0,0},{0, 0},
            },
    .alarm_s = DIS(0)|DIS(1)|DIS(2)|DIS(3)|DIS(4)|DIS(5)|DIS(6)|DIS(7)|DIS(8),
    .alarm_ss= DEN(0)|DEN(3)|DEN(6),
    .g_AlarmIsOn_next = 0,
};

/**
 * 设置闹钟时间.
 * @idx 闹钟ID
 * @ss 状态
 * @ hour, minute 闹钟时间:小时，分钟.(十进制)
 */
static int __SetAlarm(unsigned char idx, unsigned char  ss,\
	unsigned char  hour, unsigned char  minute)
{
    if ( (ss > 2) || (idx > SIZEOF(alarm.al)) )
    {
        return -1;
    }

     /* 以下修复Bug， 时分应为十进制格式，2016.11.10 */
    if ( hour > 23 || minute > 59 )
    {
        return -1;
    }

    alarm.alarm_s &= ~(0x3<<(idx<<1));
    alarm.alarm_ss &= ~(0x1<<idx);

    if ( ss == 1 )
    {
        alarm.alarm_s |= (1<<(idx<<1));
                
        alarm.al[idx].hour = hour;//((hour>>4)*10) + (hour & 0xF);
        alarm.al[idx].minute = minute;//((minute>>4)*10) + (minute & 0xF);
        
        if ( idx == 0 || idx == 3 || idx == 6 )
        {
            alarm.alarm_ss |= (0x1<<idx);
        }
    }else if ( ss == 2 )
    {
        alarm.alarm_s |= (unsigned long)(((unsigned long)(2))<<(idx<<1));
    }
    
    return 0;
}

/**
 * 为了使闹钟有多次提醒功能，这里用户没设定一个闹钟，程序额外增加两个闹钟.
 *　闹钟的间隔为３分钟和５分钟．　
 * 　buf的内容格式请参看协议文档.
 */
int SetAlarm( const unsigned char *buf )
{
    unsigned char hour, min, ss;

    for ( int i = 0; i < 3; i++ )
    {
        /* 以下修复Bug， 时分应为十进制格式，2016.11.10 */
        ss = buf[i];
        hour =  BCDToDEC( buf[5+i*2] );
        min  =  BCDToDEC( buf[5+i*2+1]) ;

        if ( __SetAlarm( i*3+0, ss, hour, min ) != 0x0 )
        {
            return -1;
        }

        min += 3;
        /* 以下修复Bug， 时分应为十进制格式，2016.11.10 */
        
        if ( min > 59 ){ min %= 60; hour += 1; hour %= 24;}

        if ( __SetAlarm( i*3+1, ss, hour, min ) != 0x0 )
        {
            return -1;
        }

        min += 5;
        /* 以下修复Bug， 时分应为十进制格式，2016.11.10 */
        
        if ( min > 59 ){ min %= 60; hour += 1; hour %= 24;}

        if ( __SetAlarm( i*3+2, ss, hour, min ) != 0x0 )
        {
            return -1;
        }
    }

    return 0;
}

/**
 * 计算距离当前时间最近的一个闹钟时间.
 * @init_flag ： 函数第一次调用时 init_flag需要设定为 1，其后调用需设置为 0
 */
void calc_next_Alarm( int init_flag )
{
    unsigned long alarm_time_to_min_today[9];
    unsigned long alarm_time_to_min_tomorrow[9];

    unsigned long alarm_time_to_min_now = current_timer.hour * 60 +
		current_timer.minute;

    for ( int i = 0; i < 9; i++ )
    {
        alarm_time_to_min_today[i] = H(i) * 60 + M(i);
        alarm_time_to_min_tomorrow[i] = H(i) * 60 + M(i) + 60 * 24;
    }

    unsigned long alarm_time_min = 0x7fffffff;
	
    for ( unsigned int i = 0; i < 9; i++ )
    {
        if ((alarm.alarm_s >> (i*2)) & 0x1)
        {
          if ( (alarm.alarm_ss >> i) & 0x1) /* 附加条件：若该闹钟没有被触摸取消 */
          {
            if (alarm_time_to_min_now < alarm_time_to_min_today[i])
            {
                if (alarm_time_min > alarm_time_to_min_today[i])
                {
                    alarm_time_min = alarm_time_to_min_today[i];
                }
            }
            
          }
        }
    }

    for ( unsigned long i = 0; i < 9; i++ )
    {
        if ((alarm.alarm_s >> (i*2)) & 0x1)
        {
          if ( (alarm.alarm_ss >> i) & 0x1) /* 附加条件：若该闹钟没有被触摸取消 */
          {          
            if (alarm_time_to_min_now < alarm_time_to_min_tomorrow[i])
            {
                if (alarm_time_min > alarm_time_to_min_tomorrow[i])
                {
                    alarm_time_min = alarm_time_to_min_tomorrow[i];
                }
            }
          }
        }
    }

    if ( (alarm.alarm_s & (AEN(0)|(AEN(1)|AEN(2)|AEN(3)|AEN(4)|AEN(5)|AEN(6)|AEN(7)|AEN(8)))) != 0 )
    {
        alarm_time_min %= 24 * 60;

        if ( init_flag == 1 )
        {
            alarm.g_Ahour_next = alarm_time_min / 60;
            alarm.g_Amin_next = alarm_time_min % 60;

            alarm.g_Amin_cur = alarm.g_Amin_next;
            alarm.g_Ahour_cur = alarm.g_Ahour_next;
        }else{

            alarm.g_Amin_cur = alarm.g_Amin_next;
            alarm.g_Ahour_cur = alarm.g_Ahour_next;

            alarm.g_Ahour_next = alarm_time_min / 60;
            alarm.g_Amin_next = alarm_time_min % 60;
        }
    }

    for( unsigned int i = 0; i < 9; i++ )
    {
        if ( alarm.g_Amin_next == M(i) &&
            alarm.g_Ahour_next == H(i) )
        {
            if ( (alarm.alarm_s >> (i*2)) & 0x1)
            {
                alarm.g_AlarmIsOn_next = 1;
            }
            else
            {
                alarm.g_AlarmIsOn_next = 0;
            }
        }
    }

    /**
     * 没有闹钟开启，默认显示00:00
     */
    if ( alarm.g_AlarmIsOn_next == 0 )
    {
        alarm.g_Ahour_next = 0;
        alarm.g_Amin_next  = 0;
    }
}

#include <string.h>

void alarm_reset(void)
{
	memset(&alarm,0,sizeof(alarm));
}

