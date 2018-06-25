#ifndef __UI_WIN_SPORT_H__
#define __UI_WIN_SPORT_H__

extern unsigned long sport_stat;
extern unsigned char notify_flag;

extern 
unsigned long win_sport_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p);

#endif
