
#include "Drink.h"
#include "ClockTick.h"


#define AEN(x) ((unsigned long)(1)<<(x*2))   /* 使能闹钟位 */

Drink drink;

/**
 * 设置闹钟时间.
 * @idx 闹钟ID
 * @ss 状态
 * @ hour, minute 闹钟时间:小时，分钟.(十进制)
 */
static int __SetDrink(unsigned char idx, unsigned char  ss,\
	unsigned char  hour, unsigned char  minute)
{
    if ( (ss > 2) || (idx > SIZEOF(drink.al)) )
    {
        return -1;
    }

     /* 以下修复Bug， 时分应为十进制格式，2016.11.10 */
    if ( hour > 23 || minute > 59 )
    {
        return -1;
    }

    drink.alarm_s &= ~(0x3<<(idx<<1));

    if ( ss == 1 )
    {
        drink.alarm_s |= (1<<(idx<<1));
                
        drink.al[idx].hour = hour;
        drink.al[idx].minute = minute;
    }else if ( ss == 2 )
    {
        drink.alarm_s |= (unsigned long)(((unsigned long)(2))<<(idx<<1));
    }
    
    return 0;
}

/**
 * 为了使闹钟有多次提醒功能，这里用户没设定一个闹钟，程序额外增加两个闹钟.
 *　闹钟的间隔为３分钟和５分钟．　
 * 　buf的内容格式请参看协议文档.
 */
int SetDrink( const unsigned char *buf )
{
    unsigned char hour, min, ss;

    for ( int i = 0; i < 5; i++ )
    {
        /* 以下修复Bug， 时分应为十进制格式，2016.11.10 */
        
        ss = buf[i];
        hour =  BCDToDEC( buf[5+i*2] );
        min  =  BCDToDEC( buf[5+i*2+1]) ;

        if ( __SetDrink( i, ss, hour, min ) != 0x0 )
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
void calc_next_Drink( int init_flag )
{
    unsigned int alarm_time_to_min_today[5];
    unsigned int alarm_time_to_min_tomorrow[5];

    unsigned int alarm_time_to_min_now = current_timer.hour * 60 +
		current_timer.minute;

    for ( int i = 0; i < 5; i++ )
    {
        alarm_time_to_min_today[i] = DH(i) * 60 + DM(i);
        alarm_time_to_min_tomorrow[i] = DH(i) * 60 + DM(i) + 60 * 24;
    }

    unsigned int alarm_time_min = 0x7fffffff;
	
    for ( int i = 0; i < 5; i++ )
    {
        if ((drink.alarm_s >> (i*2)) & 0x1)
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

    for ( int i = 0; i < 5; i++ )
    {
        if ((drink.alarm_s >> (i*2)) & 0x1)
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

    if ( (drink.alarm_s & (AEN(0)|(AEN(1)|AEN(2)|AEN(3)|AEN(4)|AEN(5)))) != 0 )
    {
        alarm_time_min %= 24 * 60;

        if ( init_flag == 1 )
        {
            drink.g_Ahour_next = alarm_time_min / 60;
            drink.g_Amin_next = alarm_time_min % 60;

            drink.g_Amin_cur = drink.g_Amin_next;
            drink.g_Ahour_cur = drink.g_Ahour_next;
        }else{

            drink.g_Amin_cur = drink.g_Amin_next;
            drink.g_Ahour_cur = drink.g_Ahour_next;

            drink.g_Ahour_next = alarm_time_min / 60;
            drink.g_Amin_next = alarm_time_min % 60;
        }
    }

    for( int i = 0; i < 5; i++ )
    {
        if ( drink.g_Amin_next == DM(i) &&
            drink.g_Ahour_next == DH(i) )
        {
            if ( (drink.alarm_s >> (i*2)) & 0x1)
            {
                drink.g_AlarmIsOn_next = 1;
            }
            else
            {
                drink.g_AlarmIsOn_next = 0;
            }
        }
    }
}

#include <string.h>

void drink_reset(void)
{
    memset(&drink, 0, sizeof(drink));
}

