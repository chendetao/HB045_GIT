#ifndef __TASK_GUI_H__
#define __TASK_GUI_H__

#define TASK_GUI_INIT_EVT                0x00000001
#define TASK_GUI_UI_EVT            		 0x00000002
#define TASK_GUI_UPDATE_EVT              0x00000010
#define TASK_GUI_TOUCH_EVT               0x00000020
#define TASK_GUI_PRESS_EVT               0x00000040
#define TASK_GUI_NOTIFY_EVT              0x00000080
#define TASK_GUI_TEST_EVT                0x00000100
#define TASK_GUI_TAKE_PHOTO_EVT          0x00000200
#define TASK_GUI_EXIT_PHOTO_EVT          0x00000400
#define TASK_UI_MINUTE_TIMEOUT_EVT       0x00000800
#define TASK_GUI_LCD_LEVEL_UPDATE_EVT    0x00001000
#define TASK_GUI_BLE_DISCONNECT_EVT      0x00002000


#define TASK_GUI_HR_START_EVT            0x00004000
#define TASK_GUI_HR_STOP_EVT             0x00008000

#define TASK_GUI_FONT_TSET_EVT           0x00010000

#define TASK_GUI_LCD_CONFIG_DELAY_EVT    0x00020000
#define TASK_GUI_SYS_POWEROFF_EVT        0x00040000

extern unsigned long taskGUITaskId;

extern unsigned long taskGUI( unsigned long task_id, unsigned long events );
extern void taskGUIInit( unsigned long task_id );

#endif
