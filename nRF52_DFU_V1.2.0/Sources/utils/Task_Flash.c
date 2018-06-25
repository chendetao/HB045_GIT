/*******************************************************************************
 * File Task_Flash.c                                                           *
 * Flash Memory Manager implementation                                         *
 * 2017.5.29                                                                   *
 * Version V1.0                                                                *
 ******************************************************************************/

#include "pstorage.h"
#include "nrf_error.h"
#include <string.h>
#include <stdio.h>

#include "OSAL_Timer.h"
#include "OSAL.h"
#include "OSAL_Task.h"

#include "Task_Ble.h"
#include "Task_Upload.h"
#include "Task_Flash.h"
#include "Task_Step.h"
#include "Task_Battery.h"

#include "UserSetting.h"
#include "ClockTick.h"
#include "btprotocol.h"
#include "sleep_algorithm.h"
#include "Alarm.h"

struct FlashMemory fm;
struct FlashMemoryControl fmc;
unsigned char swap[FM_BLOCK_SWAP_SIZE];

pstorage_module_param_t fm_block_param;
pstorage_handle_t fm_block_id;


/*******************************************************************************
 * void load_config_from_flash(void)                                           *
 * 从Flash信息页读取上一次保存的参数信息到                                     *
 * 当前工作参数                                                                *
 ******************************************************************************/
 
void load_config_from_flash(void)
{
	load_pedometer(fm.last_pedometer);
	load_date_time(&fm.last_time);
	
	config.weight = fm.weight;
	config.height = fm.height;
	config.unit = fm.unit;
	config.time_format = fm.time_format;
	config.hr_auto_enable = fm.hr_auto_enable;
	config.lcd_level_r = fm.lcd_level_r;
	config.lcd_level_g = fm.lcd_level_g;
	config.lcd_level_b = fm.lcd_level_b;
	config.age = fm.age;
	config.sex = fm.sex;
	config.notify_enable = fm.notify_enable;
	config.delay_timeout = fm.delay_timeout;
		
	power.time_after_last = fm.time_after_last;
	
	AutoSleep.out_TotalSleepTime = fm.autoSleepTime;	
}

/*******************************************************************************
 * void store_config_default(void)                                             *
 * 将默认工作参数复制到fm结构体,以便保存到Flash                                *
 ******************************************************************************/

void store_config_default(void)
{
	fm.last_pedometer = 0;
	fm.weight = DEFAULT_WEIGHT;
	fm.height = DEFAULT_HEIGHT;
	fm.unit   = 0;
	fm.time_format = 0;
	fm.sex    = 0;
	fm.age    = DEFAULT_AGE;

	fm.last_time.year  = DEFAULT_YEAR;
	fm.last_time.month = DEFAULT_MONTH;
	fm.last_time.day   = DEFAULT_DAY;
	fm.last_time.hour  = DEFAULT_HOUR;
	fm.last_time.minute = DEFAULT_MINUTE;
	fm.last_time.second = DEFAULT_SECOND;
	fm.last_time.week  = DEFAULT_WEEK;
	
	fm.time_after_last = 0;
	fm.hr_auto_enable  = 0;
	fm.lcd_level_r = LCD_LEVEL_R;
	fm.lcd_level_g = LCD_LEVEL_R;
	fm.lcd_level_b = LCD_LEVEL_R;
	
	fm.notify_enable = 0; /* 默认功能关闭 */
	fm.delay_timeout = 0;
	
	fm.autoSleepTime = 0;/* 默认值 */
}

/*******************************************************************************
 * void store_current_config(void)                                             *
 * 将当前工作参数复制到fm结构体,以便保存到Flash                                *
 ******************************************************************************/

void store_current_config(void)
{
	fm.last_pedometer = pedometer.counter;
	fm.weight = config.weight;
	fm.height = config.height;
	fm.unit   = config.unit;
	fm.time_format = config.time_format;
	fm.time_after_last = power.time_after_last;
	fm.hr_auto_enable  = config.hr_auto_enable;
	
	fm.lcd_level_r = config.lcd_level_r;
	fm.lcd_level_g = config.lcd_level_g;
	fm.lcd_level_b = config.lcd_level_b;
	fm.sex = config.sex;
	fm.age = config.age;
	
	fm.notify_enable = config.notify_enable;
	fm.delay_timeout = config.delay_timeout;
	
	memcpy( &fm.last_time, getSystemTimer(), sizeof(fm.last_time));
	
	fm.autoSleepTime = AutoSleep.out_TotalSleepTime;	
}

/*******************************************************************************
 * static void flash_memofy_cb_handler( pstorage_handle_t *handle,             *
 *	unsigned char op_code, unsigned int result, unsigned char *p_data,         *
 *	unsigned int data_len )                                                    *
 * Flash操作结果回调函数，由BLE内核抛出事件                                    *
 ******************************************************************************/

static void flash_memofy_cb_handler( pstorage_handle_t *handle,
	unsigned char op_code, unsigned int result, unsigned char *p_data,
	unsigned int data_len )
{
	int inBlock = 0, idx = 0;
	
	if ( handle->module_id != fm_block_id.module_id )
	{ 
		fmc.index_error++;
		return; 
	}
	
	for ( int i = 0; i < FM_BLOCK_NUMBER; i++ )
	{
		if (handle->block_id == fmc.block_id[i].block_id )
		{
            inBlock = 1;
			idx = i;
			break;
		}
	}
	
	if ( inBlock == 0 )
	{
		fmc.index_error++;
		return;		
	}
	
	switch ( op_code )
	{
	case PSTORAGE_STORE_OP_CODE:
		if ( result == NRF_SUCCESS )
		{
		}
		break;
	case PSTORAGE_UPDATE_OP_CODE:
		if ( result != NRF_SUCCESS )
		{
			break;
		}
		{	/**
			 * get the index for flash memory block.
			 * and update information structure.
			 */
			fmc.updated_cnt[idx]++;
			
			if ( idx == 0 ) break;
			
			/** Wrteen done, update the control structure, do
			 * with TASK_FLASH_UPDATE_EVT 
			 */
			flash_memory_update(idx);
			osal_set_event ( taskFlashTaskId, TASK_FLASH_UPDATE_EVT );
		};
		break;
	case PSTORAGE_CLEAR_OP_CODE:
		break;
	default:
		break;	
	}
	
	if ( op_code == PSTORAGE_UPDATE_OP_CODE ){
		fmc.lock = 0;  /** Unload the write operation */
		
		if ( idx == 0 )
		fmc.lock_mticks = osal_systemClock - fmc.lock_mticks;
	}
}

/*******************************************************************************
 *  原型 int flash_memory_Init(void)                                           *
 *  功能 像内核注册一块内存区域, 并且读取该内存块中的配置信息,                 *
 *       根据配置信息是否有效来决定使用保存的配置信息或者默认配置.             *
 *  参数 无                                                                    *
 *  返回 成功 0, 失败(-1)                                                      *
 ******************************************************************************/

int flash_memory_Init(void)
{
	int retval;
	pstorage_handle_t id;
	
	fmc.swap = swap;
	memset(fmc.faild_r_lock_cnt,0, sizeof(fmc.faild_r_lock_cnt));
	memset(fmc.faild_w_lock_cnt,0, sizeof(fmc.faild_w_lock_cnt));
	fmc.lock = 0;
	fmc.put_error_cnt = 0;
	fmc.get_error_cnt = 0;
	fmc.index_error = 0;
	
	fm_block_param.block_count = FM_BLOCK_NUMBER;
	fm_block_param.block_size  = 4096;
	fm_block_param.cb = flash_memofy_cb_handler; 

	retval = pstorage_register(&fm_block_param, &fm_block_id);
	if ( retval != NRF_SUCCESS )
	{
		return -1;
	}
	
	for ( int i = 0; i < FM_BLOCK_NUMBER; i++ )
	{
		retval = pstorage_block_identifier_get(&fm_block_id, i, &id);
		if ( retval != NRF_SUCCESS )
		{
			return -1;
		}	

		fmc.block_id[i] = id;
	}
	
	retval = pstorage_block_identifier_get(&fm_block_id, 0, &id);
	if ( retval != NRF_SUCCESS )
	{
		return -1;
	}
	
	retval = pstorage_load( fmc.swap, &id, 4096, 0);
	if ( retval != NRF_SUCCESS )
	{
		return -1;
	}
	
	/**
	 * Note :
	 * pstorage_store/pstorage_update的区别
	 * 
	 * pstorage_store() 只管写入，不擦除,大于1Page,则只写入1Page
	 * pstorage_update() 写之前擦除, 若写的数据不是页对齐,则多余的数据会用Swap区
	 * 为了使提高效率,写入的数据全部按照页对齐方式，避免使用Swap
	 */
		
	if ( ((struct FlashMemory*)swap)->control_magic != 0x8A7B43C3 )
	{
		/**
		 * TODO : 在此将所有需要的设置全部设置为默认状态.
		 */
		fm.control_magic = 0x8A7B43C3;		
		memset(fm.erea, 0, sizeof(fm.erea));
		store_config_default();
		
		memcpy( fmc.swap, &fm, sizeof(struct FlashMemory));
		
		osal_start_timerEx( taskFlashTaskId, TASK_FLASH_UPDATE_EVT, 3000);
	} else
	{		
		if ( (fm.last_pedometer == 0xFFFFFFFF) || (fm.last_pedometer > 99999) )
		{
			memset(fm.erea, 0, sizeof(fm.erea));
			store_config_default();
		
			memcpy( fmc.swap, &fm, sizeof(struct FlashMemory));
		
			osal_start_timerEx( taskFlashTaskId, TASK_FLASH_UPDATE_EVT, 3000);			
		} else {
			memcpy( &fm, swap, sizeof(struct FlashMemory));		
			load_config_from_flash();
		}
	}	

	memset(fmc.updated_cnt, 0, sizeof(fmc.updated_cnt));
		
	return 0;
}

/*******************************************************************************
 * int flash_memory_put(int fm_identifier,const unsigned char *buf, int length)*
 * Function: Add a data item to the Flash Memory.                              *
 * param:                                                                      *
 *  @ fm_identifier:  the flash block erea identifier.                         *
 *  @ buf the buffer: that store data.                                         *
 *  & length: data buffer length                                               *
 ******************************************************************************/

int flash_memory_put(int fm_identifier, const unsigned char *buf, int length )
{
	pstorage_handle_t id;
	int retval, offset;
	
	if ( fmc.lock == 1 ){
		fmc.faild_w_lock_cnt[fm_identifier]++;
		return (-1);
	}
	
	fmc.lock = 1;
	
	if ( length > FM_ITEM_SIZE )
	{
		return (-1);
	}
	
	retval = pstorage_block_identifier_get(&fm_block_id, fm_identifier, &id);
	if ( retval != NRF_SUCCESS )
	{
		return (-1);
	}

	/** Read */
	retval = pstorage_load( fmc.swap, &id, FM_BLOCK_SWAP_SIZE, 0);
	if ( retval != NRF_SUCCESS )
	{
		return (-1);
	}	
	
	/* Modify */
	offset = fm.erea[fm_identifier].w_offset;

	memcpy( fmc.swap + offset*FM_ITEM_SIZE, buf, length );

	/* Update */
	retval = pstorage_update( &id, fmc.swap,  FM_BLOCK_SWAP_SIZE , 0);
	if ( retval != NRF_SUCCESS )
	{
		return (-1);
	}	

	
	return 0;
}

void flash_memory_update( int fm_identifier )
{	
	fm.erea[fm_identifier].w_offset++;
	fm.erea[fm_identifier].w_offset %= FM_BLOCK_SWAP_SIZE/FM_ITEM_SIZE;
	
	if ( fm.erea[fm_identifier].w_offset == fm.erea[fm_identifier].r_offset )
	{
		fm.erea[fm_identifier].r_offset++;
		fm.erea[fm_identifier].r_offset %= FM_BLOCK_SWAP_SIZE/FM_ITEM_SIZE;
	}
	
	fm.erea[fm_identifier].items = 
		(fm.erea[fm_identifier].items < (FM_ITEM_NUMBER-1) ) ? 
		(fm.erea[fm_identifier].items+1):((FM_ITEM_NUMBER-1));
}

int flash_memory_control_update(void)
{
	pstorage_handle_t id;
	int retval;
	
	if ( fmc.lock == 1 ){
		fmc.faild_w_lock_cnt[FM_ID_CONTROL]++;
		return (-1);
	}
	
	fmc.lock = 1;	
	
	retval = pstorage_block_identifier_get(&fm_block_id, FM_ID_CONTROL, &id);
	if ( retval != NRF_SUCCESS )
	{
		fmc.update_error_cnt++;
		return (-1);
	}	

	retval = pstorage_load( fmc.swap, &id, 4096, 0);
	if ( retval != NRF_SUCCESS )
	{
		fmc.update_error_cnt++;
		return -1;
	}	
	
	memcpy( fmc.swap, &fm, sizeof(struct FlashMemory));
	
	retval = pstorage_update( &id, fmc.swap, 4096 , 0);
	if ( retval != NRF_SUCCESS )
	{
		fmc.update_error_cnt++;
		return (-1);
	}	

	fmc.lock = 1;
	fmc.lock_mticks = osal_systemClock;

	return 0;
}

int flash_memory_reset(void)
{
	memset( &fm, 0, sizeof(fm));
	
	fm.control_magic = 0xFFFFFFFF;
	
	return flash_memory_control_update();
}

/*******************************************************************************
 * int flash_memory_get( int fm_identifier, unsigned char *buf, int length )   *
 * Function: get a data item from Flash Memory.                                *
 * param:                                                                      *
 *  @ fm_identifier:  the flash block erea identifier.                         *
 *  @ buf the buffer: to store the data.                                       *
 *  & length: data buffer length                                               *
 ******************************************************************************/

int flash_memory_get( int fm_identifier, unsigned char *buf, int length, bool update)
{
	pstorage_handle_t id;
	int retval, offset;
	
	if ( fmc.lock == 1 ){
		fmc.faild_r_lock_cnt[fm_identifier]++;
		return (-1);
	}	
	
	if ( fm.erea[fm_identifier].items == 0 )
	{
		 return (-1); /** no element */
	}	
	
	retval = pstorage_block_identifier_get(&fm_block_id, fm_identifier, &id);
	if ( retval != NRF_SUCCESS )
	{
		return -1;
	}

	/** Read */
	retval = pstorage_load( fmc.swap, &id, FM_BLOCK_SWAP_SIZE, 0);
	if ( retval != NRF_SUCCESS )
	{
		return -1;
	}	
	
	/** Copy */
	offset = fm.erea[fm_identifier].r_offset;
	
	memcpy( buf, fmc.swap + offset*FM_ITEM_SIZE, length );
		
	// if ( fm_identifier != FM_ID_SLEEP )
	{
		fm.erea[fm_identifier].r_offset++;
		fm.erea[fm_identifier].r_offset %= FM_BLOCK_SWAP_SIZE/FM_ITEM_SIZE;
	            
		fm.erea[fm_identifier].items--;
	}
	
	/* To update the control block */
	
	if ( update == true )
	{
		osal_set_event( taskFlashTaskId, TASK_FLASH_UPDATE_EVT );
	}
	
	return 0;
}

unsigned long taskFlashTaskId;

#define B_BUF_SIZE  16

typedef struct
{
	unsigned long f_id;
	unsigned char buf[16];
}Flash_Element;

typedef struct
{
	unsigned long h, t;
	Flash_Element element[B_BUF_SIZE];
}Flash_Queue;

Flash_Queue fq;

void flash_temp_queue_put(unsigned int id, unsigned char *buf, int length )
{
	memcpy(fq.element[fq.t].buf, buf, length);
	fq.element[fq.t].f_id = id;
	
	fq.t++; fq.t %= B_BUF_SIZE;
	if ( fq.t == fq.h )
	{
		fq.h++; fq.h %= B_BUF_SIZE;
	}
	
	osal_set_event( taskFlashTaskId, TASK_FLASH_QUEUE_PUT_EVT );
}

unsigned char data_clear_action_done[20] = {0x0F,0x0B,0x00};
unsigned char flash_reset_action_done[20] = {0x0F,0x1A,0x00};

unsigned long taskFlash( unsigned long task_id, unsigned long events )
{ 
	if ( events & TASK_FLASH_INIT_EVT )
	{
		flash_memory_Init();
		/** 
		 * h = t = 0;
		 */
		memset(&fq, 0, sizeof(fq));
		
		return ( events ^ TASK_FLASH_INIT_EVT );
	}
	
	if ( events & TASK_FLASH_LOAD_CONFIG_EVT )
	{
		load_config_from_flash();
	
		bt_protocol_tx( data_clear_action_done, 20);
		
		flash_memory_control_update();
		
		return ( events ^ TASK_FLASH_LOAD_CONFIG_EVT );
	}
	
	/** 
	 * 清除Flash 数据，所有配置信息设置为默认状态
	 * --->不重启 */
	if ( events & TASK_FLASH_MAKE_TO_DEFAULT_EVT )
	{
		memset(fm.erea, 0, sizeof(fm.erea));  /** 所有存储块指针归零:即存储数据无效 */
		store_config_default();               /** 默认参数保存到Flash */
		
		if ( flash_memory_control_update() != 0 )        /** 刷新Flash控制块 */
		{
			osal_start_timerEx( task_id, TASK_FLASH_MAKE_TO_DEFAULT_EVT, 1000);		
			return ( events ^ TASK_FLASH_MAKE_TO_DEFAULT_EVT );
		}
		
		/** 当前步数清零 */
		osal_set_event ( taskStepTaskId, TASK_STEP_CLEAR_ALL_EVT );
		/** 睡眠数据清零 */
		osal_set_event ( taskStepTaskId, STEP_TASK_CLEAR_SLEEP_EVT );
		/** 闹钟清零 */
		alarm_reset();
		
		/** 1秒后加载Flash配置到当前工作配置 */	
		osal_start_timerEx( task_id, TASK_FLASH_LOAD_CONFIG_EVT, 1000); 
		
		return ( events ^ TASK_FLASH_MAKE_TO_DEFAULT_EVT );
	}

	/** 
	 * 直接将用于Flash设置为无效,从而达到恢复出厂的功能
	 * --->重启 */	
	if ( events & TASK_FLASH_RESET_EVT )
	{
		if ( flash_memory_reset() != 0 )
		{
			osal_start_timerEx( task_id, TASK_FLASH_RESET_EVT,1000); 
		}else{
			bt_protocol_tx( flash_reset_action_done, 20 );
			
			osal_start_timerEx( taskBleTaskId, TASK_BLE_SYSTEM_REBOOT_EVT, 5000);
		}
		
		return ( events ^ TASK_FLASH_RESET_EVT );
	}
	
	if ( events & TASK_FLASH_UPDATE_EVT )
	{
        flash_memory_control_update();
		
		return ( events ^ TASK_FLASH_UPDATE_EVT );
	}
	
	if ( events & TASK_FLASH_UPDATE_CONFIG_EVT )
	{
		store_current_config();
		
		osal_set_event ( task_id, TASK_FLASH_UPDATE_EVT );
		
		return ( events ^ TASK_FLASH_UPDATE_CONFIG_EVT );
	}
	
	if ( events & TASK_FLASH_PUT_TEST_EVT )
	{
		unsigned char buf[16];
		
		buf[0] = DECToBCD(TimerYY());
		buf[1] = DECToBCD(TimerMM());
		buf[2] = DECToBCD(TimerDD());
		buf[3] = DECToBCD(TimerHH());
		buf[4] = DECToBCD(TimerMI());
		buf[5] = DECToBCD(TimerSE());
		buf[6] = hr.hr = osal_systemClock & 0xFF;
		 
		flash_temp_queue_put(FM_ID_HR, buf,16);
		osal_start_timerEx ( taskUploadTaskId, TASK_UPLOAD_HR_EVT, 500);
		
		osal_start_timerEx( task_id, TASK_FLASH_PUT_BP_EVT, 2000);
		
		return ( events ^ TASK_FLASH_PUT_TEST_EVT );
	}
	
	if ( events & TASK_FLASH_PUT_BP_EVT )
	{
		unsigned char buf[16];
		
		buf[0] = DECToBCD(TimerYY());
		buf[1] = DECToBCD(TimerMM());
		buf[2] = DECToBCD(TimerDD());
		buf[3] = DECToBCD(TimerHH());
		buf[4] = DECToBCD(TimerMI());
		buf[5] = DECToBCD(TimerSE());
		buf[6] = hr.hp = osal_systemClock & 0xFF;
		buf[7] = hr.lp = hr.hp - 10;
		 
		flash_temp_queue_put(FM_ID_B_PRESSURE, buf,16);
		
		osal_start_timerEx ( taskUploadTaskId, TASK_UPLOAD_B_PRESSURE_EVT, 500);

		osal_start_timerEx( task_id, TASK_FLASH_PUT_TEST_EVT, 4000);
		
		return ( events ^ TASK_FLASH_PUT_BP_EVT );
	}
	
	if ( events & TASK_FLASH_GET_TEST_EVT )
	{
		unsigned char buf[16] = {0,};
		
		flash_memory_get(FM_ID_HR, buf,16, true);
			
		if ( buf[0] != 0xcc )
		{
			 while(1){__asm("NOP");};
		}
		
		return ( events ^ TASK_FLASH_GET_TEST_EVT );
	}
	
	if ( events & TASK_FLASH_QUEUE_PUT_EVT )
	{		
		if ( fq.h != fq.t )
		{
			if ( flash_memory_put( fq.element[fq.h].f_id,
				fq.element[fq.h].buf, 16) == 0 )
			{
				 fq.h++; fq.h %= B_BUF_SIZE;
			}
		}
		
		if ( fq.h != fq.t )
		{
			osal_start_timerEx( task_id, TASK_FLASH_QUEUE_PUT_EVT, 800);
		}
		
		return ( events ^  TASK_FLASH_QUEUE_PUT_EVT );
	}
	
	return 0;
}

void taskFlashInit( unsigned long task_id )
{
	taskFlashTaskId = task_id;
	
	osal_set_event ( task_id, TASK_FLASH_INIT_EVT );
}
