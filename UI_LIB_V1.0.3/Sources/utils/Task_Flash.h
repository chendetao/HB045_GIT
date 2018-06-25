#ifndef __TASK_FLASH_H__
#define __TASK_FLASH_H__

#include <stdbool.h>
#include "ClockTick.h"
#include "pstorage.h"
#include "UserSetting.h"

#define FM_ID_CONTROL   				0
#define FM_ID_PEDOMETER     			1	
#define FM_ID_SLEEP         			2
#define FM_ID_HR            			3
#define FM_ID_B_PRESSURE    			4
#define FM_ID_SPO2                      5
#define FM_ID_SEG_PEDO                  6
#define FMC_ID_RUNNING                  7

#define FM_BLOCK_NUMBER                 8
#define FM_BLOCK_SWAP_SIZE              4096
#define FM_ITEM_SIZE                    16
#define FM_ITEM_NUMBER   (FM_BLOCK_SWAP_SIZE/FM_ITEM_SIZE) 

#define FLASH_LOCKED  1
#define FLASH_NULL    2

struct EreaInfo
{
	unsigned int magic;
	unsigned int w_offset;
	unsigned int r_offset;
	unsigned int items;
};

struct FlashMemory
{
	unsigned int control_magic;			  /** 控制块有效标识 */
	struct EreaInfo erea[FM_BLOCK_NUMBER];/** 子块信息 */
	unsigned long last_pedometer;         /** 最后一次有效步数 */
	unsigned int time_after_last;         /** 上一次充电到目前的时间 */
	struct time_struct last_time;         /** 日期时间信息 */
	unsigned char weight;				/** 体重 */
	unsigned char height;               /** 升高 */
	unsigned char age;                  /** 年龄 */
	unsigned char sex;                  /** 性别 */
	unsigned char unit;                 /** 公制单位,英制单位设置 */
	unsigned char time_format;          /** 时间格式(24/12) */
	unsigned char hr_auto_enable;		/** 自动测量心率 */
	unsigned char lcd_level_r;          /** LCD对比度控制 R */
	unsigned char lcd_level_g;          /** LCD对比度控制 G */
	unsigned char lcd_level_b;          /** LCD对比度控制 B */
	unsigned char notify_enable;        /** 防丢提醒使能位 */
	unsigned char delay_timeout;        /** 防丢提醒延时时间 */
	unsigned long autoSleepTime;        /** 总的睡眠时间 */
	
	char device_name[DEVICE_NAME_LENGTH+1];	
#if (OTA_MODE_ENABLE==0)	
	unsigned char test_mode;            /** 测试模式 */
#endif	
};

struct FlashMemoryControl
{
	unsigned long updated_cnt[FM_BLOCK_NUMBER];   	/** Update成功的次数 */
	unsigned long lock;
	unsigned long faild_r_lock_cnt[FM_BLOCK_NUMBER];  /* 子块解锁失败次数 */
	unsigned long faild_w_lock_cnt[FM_BLOCK_NUMBER];  /* 子块解锁失败次数 */
	unsigned char *swap;                /** Swap区地址 */
	int put_error_cnt;
	int get_error_cnt;
	int update_error_cnt;
	int index_error;
	pstorage_handle_t block_id[FM_BLOCK_NUMBER];
	unsigned long lock_mticks;
};

extern struct FlashMemory fm;
extern struct FlashMemoryControl fmc;

extern int flash_memory_Init(void);
extern int flash_memory_put(int fm_identifier, const unsigned char *buf, int length );
extern int flash_memory_get( int fm_identifier, unsigned char *buf, int length, bool update);
void flash_memory_update( int fm_identifier );
int flash_memory_control_update(void);
extern int flash_memory_reset(void);
extern void flash_temp_queue_put(unsigned int id, unsigned char *buf, int length );

extern void store_current_config(void);

#define TASK_FLASH_INIT_EVT        					0x00000001
#define TASK_FLASH_UPDATE_EVT      					0x00000002
#define TASK_FLASH_PUT_TEST_EVT    					0x00000004
#define TASK_FLASH_GET_TEST_EVT    					0x00000008
#define TASK_FLASH_PUT_BP_EVT      					0x00000010
#define TASK_FLASH_UPDATE_CONFIG_EVT 				0x00000020

#define TASK_FLASH_QUEUE_PUT_EVT        			0x00000040

#define TASK_FLASH_RESET_EVT            			0x00000080
#define TASK_FLASH_MAKE_TO_DEFAULT_EVT              0x00000100
#define TASK_FLASH_LOAD_CONFIG_EVT                  0x00000200

extern unsigned long taskFlashTaskId;
extern unsigned long taskFlash( unsigned long task_id, unsigned long events );
extern void taskFlashInit( unsigned long task_id );

#endif
