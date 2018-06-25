
#ifndef __UI_WIN_ALARM_H__
#define __UI_WIN_ALARM_H__

extern 
unsigned long win_alarm_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p);

void show_alarm(int x, int y, int hour, int minute );

#endif
