#ifndef __CLOCK_TICK_H__
#define __CLOCK_TICK_H__

#define DEFAULT_YEAR   		0x11
#define DEFAULT_MONTH  		0x08
#define DEFAULT_DAY    		0x05
#define DEFAULT_HOUR     	0x08
#define DEFAULT_MINUTE   	0x00
#define DEFAULT_SECOND   	0x00

#define DEFAULT_WEEK     	0x06

struct time_struct{
	unsigned char hour;
    unsigned char minute;
    unsigned char second;
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char week;
    unsigned char fill;
};

#define BCDToDEC(x) ((((x)>>4) * 10) + ((x) & 0xF))
#define DECToBCD(x) ((((x)/10)<<4) | ((x) % 10))

#define TimerYY() (getSystemTimer()->year)
#define TimerMM() (getSystemTimer()->month)
#define TimerDD() (getSystemTimer()->day)
#define TimerHH() (getSystemTimer()->hour)
#define TimerMI() (getSystemTimer()->minute)
#define TimerSE() (getSystemTimer()->second)
#define TimerWK() (getSystemTimer()->week)

extern void system_clock_click(unsigned long seconds );
extern void setSystemTimer(const struct time_struct *new_timer);

extern struct time_struct current_timer;
extern struct time_struct last_timer;

#define getSystemTimer() (&current_timer)

extern void calc_next_Alarm( int init_flag );

extern int SetDate( unsigned char year, unsigned char month, unsigned char day,
                      unsigned char week);
extern int SetTimer( unsigned char hour, unsigned char minute, unsigned char second);

#define timeDiff(a,b) ((a[0]!=b[0])||(a[1]!=b[1])||(a[2] != b[2])\
                        ||(a[3] != b[3])||(a[4] != b[4]) )

extern void load_date_time(const struct time_struct *new_timer);

extern unsigned long getUTC(void);
extern unsigned long Convert_UTC_time(struct time_struct *t);
extern void Cal_Time(unsigned long Count,struct time_struct *Time);

#endif
