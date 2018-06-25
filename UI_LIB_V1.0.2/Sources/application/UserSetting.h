#ifndef __USER_SETTING_H__
#define __USER_SETTING_H__

#define BSTOTW_FLAG_ENABLE 				0x1
#define BSTOTW_FLAG_TURN_OVER_ENABLE 	0x2
#define BSTOTW_FLAG_TEMP_ENABLE 		0x4
#define BSTOTW_FLAG_CC_ENABLE 	  	    0x8

#define LCD_LEVEL_R          		 	0xDC
#define LCD_LEVEL_G           			0xDC
#define LCD_LEVEL_B           			0xDC

#define DEFAULT_AGE                     25
#define DEFAULT_WEIGHT                  60
#define DEFAULT_HEIGHT                 160

#define DEVICE_NAME_LENGTH               7
#define DEVICE_NAME_CUSTOM              "F07"

#define OTA_MODE_ENABLE					0

typedef struct      /* 使用位域，是为了节约内存，结构统一 */
{
    unsigned char viborate_enable:1;
    unsigned char sleepEnable:1;
    unsigned char action_occur:1;
    unsigned char alarm_next:1;  
    unsigned char hr_auto_enable:4;                  
    unsigned char autoSleepEnable;
	unsigned char weight;
	unsigned char height;
	unsigned char age;
	unsigned char sex;
	unsigned char notify_enable:1;
	unsigned char delay_timeout:7;
	unsigned char control;
	unsigned char lcdEnable;
    unsigned char enable;
}Configure_Typef;

extern Configure_Typef config;

#endif
