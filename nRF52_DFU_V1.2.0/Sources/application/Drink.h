#ifndef __DRINK_H__
#define __DRINK_H__

#define SIZEOF(x) (sizeof(x)/sizeof(x[0]))

#include "Alarm.h"

typedef struct
{
   Alarm_Typef al[5];
   unsigned long alarm_s;
   unsigned char g_Ahour_next;
   unsigned char g_Amin_next;
   unsigned char g_Ahour_cur;
   unsigned char g_Amin_cur;
   unsigned char g_AlarmIsOn_next;
}Drink;

#define DH(idx) (drink.al[idx].hour)
#define DM(idx) (drink.al[idx].minute)

extern Drink drink;
extern int SetDrink( const unsigned char *buf );
extern void calc_next_Drink( int init_flag );
extern void drink_reset(void);

#endif

