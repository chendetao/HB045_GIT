#include "OSAL_Timer.h"
#include "OSAL.h"
#include "Task_GUI.h"
#include "Task_Step.h"
#include "Task_Battery.h"
#include "Task_Upload.h"
#include "Task_Flash.h"
#include "Task_Store.h"
#include "Task_Ble.h"
#include "Task_Hr.h"

#include "HeartComp.h"
#include "ClockTick.h"

struct time_struct current_timer = { DEFAULT_HOUR,DEFAULT_MINUTE,DEFAULT_SECOND,
                             DEFAULT_YEAR,DEFAULT_MONTH,DEFAULT_DAY,
                             DEFAULT_WEEK }; /* BCD 2016.9.20 DEX 08:00:00 */

const unsigned int mtable[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

struct time_struct last_timer;

void system_clock_click(unsigned long seconds )
{    

}

void setSystemTimer(const struct time_struct *new_timer)
{

}

int SetTimer( unsigned char hour, unsigned char minute, unsigned char second)
{
    return 0;
}

int SetDate( unsigned char year, unsigned char month, unsigned char day,
                      unsigned char week)
{
    return 0;
}

void load_date_time(const struct time_struct *new_timer)
{
}

#include "time.h"

unsigned long getUTC(void)
{	
	unsigned long utc;
	
	return utc;
}

unsigned long Convert_UTC_time(struct time_struct *t)
{
	unsigned long utc;

	return utc;
}

