#ifndef __UI_WIN_NOTIFY_H__
#define __UI_WIN_NOTIFY_H__

#define NOTIFY_ID_INVALID   0
#define NOFIFY_ID_CALLING   1
#define NOTIFY_ID_EMAIL     2
#define NOTIFY_ID_APP_MSG   3   /* Now using the NOTIFY_ID_EMAIL to replace */
#define NOTIFY_ID_ALARM     4
#define NOTIFY_ID_FINDME    5   /* Phone find me */
#define NOTIFY_ID_BLE       6
#define NOTIFY_ID_SPORT     7   /* 久坐提醒 */
#define NOTIFY_ID_FACEBOOK  8
#define NOTIFY_ID_WHATSAPP  9
#define NOTIFY_ID_TOINDEX(id) ((id)-1)


extern unsigned long win_notify_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p);

#endif
