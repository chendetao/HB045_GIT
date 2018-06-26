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
	unsigned char shutdown;
	unsigned char unit;
	unsigned char time_format;
	unsigned char hrWaveUploadEnable;
	unsigned char lcd_level_r;
	unsigned char lcd_level_g;
	unsigned char lcd_level_b;
	unsigned char ui_style;
	unsigned char enable;
	unsigned char hflag;
	unsigned char sleep_mode,last_sleep_mode;
    unsigned char gsensor_debug;
	unsigned char gsensor_fast_enable;	
	unsigned char showFlag;
	unsigned char sync_flag;	
	unsigned char motor_test_enable;
    unsigned char app_all_enable;
    
    unsigned long train_running_type;
    unsigned long train_running_utc;
    unsigned long train_running_pedometer;
    unsigned long train_running_calorie;
    unsigned long train_running_distance;
    unsigned long train_bycle_utc;
    unsigned long train_bycle_pedometer;
    unsigned long train_bycle_distance;
    unsigned long train_bycle_calorie;
    unsigned long train_sw_utc;
    unsigned long train_sw_pedometer;
    unsigned long train_sw_distance;
    unsigned long train_sw_calorie;    
}Configure_Typef;

extern Configure_Typef config;

typedef struct
{
    unsigned char  ss;
    unsigned char  interval;  /* 提醒间隔：单位5分钟 */
    unsigned short sedentary_start;
    unsigned short sedentary_end;
    unsigned short start;
    unsigned short end;
    unsigned short pedometer;
	unsigned long  last_pedometer;
}SportMonitor_Typdef;

extern SportMonitor_Typdef monitor;


typedef enum
{
	HR_DECT_STAT_NONE = 1,
	HR_DECT_STAT_DECTING,
	HR_DECT_STAT_NORMAL,
	HR_DECT_STAT_ERROR,
	HR_DECT_STAT_DONE,
	HR_DECT_STAT_STOP,
}hr_stat;

typedef struct
{
	unsigned char hr;
	unsigned char lp;
	unsigned char hp;
	unsigned char stop;
	unsigned char leave_flag;	
	unsigned char sp_h;
    unsigned char sp_l;	
	hr_stat stat;
	unsigned short fg:1;
	unsigned short auto_flag:1;
	unsigned short auto_measure_enable:1;
	unsigned short auto_measure_interval:12;
	unsigned short touch_level;
#if (OTA_MODE_ENABLE==0)	
	unsigned char ok;	
#endif	
}HerateTypdef;

extern HerateTypdef hr;

#define ANCS_NOTIFY_ENABLE_CALL    		0x0001
#define ANCS_NOTIFY_ENABLE_SMS     		0x0004
#define ANCS_NOTIFY_ENABLE_QQ      		0x0008
#define ANCS_NOTIFY_ENABLE_WX      		0x0010
#define ANCS_NOTIFY_ENABLE_WHATAPP 		0x0020
#define ANCS_NOTIFY_ENABLE_FACEBOOK 	0x0040
#define ANCS_NOTIFY_ENABLE_LINE        	0x0080
#define ANCS_NOTIFY_ENABLE_TT         	0x0100
#define ANCS_NOTIFY_ENABLE_SKYPE      	0x0200
#define ANCS_NOTIFY_ENABLE_INS        	0x0400
#define ANCS_NOTIFY_ENABLE_GMAIL        0x0800

typedef struct{
	unsigned char isConnected;
	unsigned long ancs_notify_enable;
}BLE_Typdef;

extern BLE_Typdef ble;
extern unsigned char gsensor_ok;
extern unsigned char update_flag;
extern unsigned char ancs_ancs_flag;

#endif
