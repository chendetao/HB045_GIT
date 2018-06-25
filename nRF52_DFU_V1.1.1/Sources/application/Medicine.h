#ifndef __MEDICINE_H__
#define __MEDICINE_H__

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
}Medicine;

#define MMH(idx) (medicine.al[idx].hour)
#define MMM(idx) (medicine.al[idx].minute)

extern Medicine medicine;
extern int SetMedicine( const unsigned char *buf );
extern void calc_next_Medicine( int init_flag );
extern void drink_reset(void);

#endif

