#ifndef __ALARM_H__
#define __ALARM_H__

#define SIZEOF(x) (sizeof(x)/sizeof(x[0]))

typedef struct
{
   unsigned char hour;
   unsigned char minute;
}Alarm_Typef;

typedef struct
{
   Alarm_Typef al[9];
   unsigned long alarm_s;
   unsigned long alarm_ss;/* 附加标识，用于标识被用户按键触摸取消的闹钟 */
   unsigned char g_Ahour_next;
   unsigned char g_Amin_next;
   unsigned char g_Ahour_cur;
   unsigned char g_Amin_cur;
   unsigned char g_AlarmIsOn_next;
}Alarm;

#define H(idx) (alarm.al[idx].hour)
#define M(idx) (alarm.al[idx].minute)

extern Alarm alarm;
extern int SetAlarm( const unsigned char *buf );
extern void alarm_reset(void);

#endif
