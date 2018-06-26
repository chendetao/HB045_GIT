#ifndef __UI_INFO_ICON_H__
#define __UI_INFO_ICON_H__

#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_GUI.h"
#include "Task_Step.h"
#include "LCD_ST7735.h"

#include "string.h"
#include "Window.h"
#include "UserSetting.h"
#include "UI_Draw.h"

extern unsigned long win_info_proc(unsigned long wid, unsigned long msg_type, 
                            unsigned long msg_param, unsigned long *p);

#endif
