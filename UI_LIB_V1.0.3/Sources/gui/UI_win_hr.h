#ifndef __UI_WIN_HR_H__
#define __UI_WIN_HR_H__

extern unsigned long win_hr_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p);

extern void show_error(int x, int y);
extern void show_ready(void);

#endif
