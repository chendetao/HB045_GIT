#include "OSAL.h"
#include "OSAL_Timer.h"
#include "OSAL_Task.h"

#include "Task_GUI.h"
#include "Window.h"

#include "LCD_ST7735.h"
#include "Button.h"
#include "UI_win_notify.h"
#include "F_Lib.h"
#include "UserSetting.h"
#include "ClockTick.h"
#include <string.h>
#include "UI_lib.h"
#include "SPI_HW.h"
#include "HFMA2Ylib.h"

void WritePage( unsigned long addr, unsigned char *buf, int len );
void EraseSector( unsigned long addr );
void ReadPage( unsigned long addr, unsigned char *buf, int len );

unsigned long taskGUITaskId;

unsigned long Flash_stat = 0;
unsigned long erase_All_page = 0;
unsigned long erase_Cur_page = 0;

unsigned long write_All_page = 0;
unsigned long write_Cur_page = 0;

unsigned long read_All_page = 0;
unsigned long read_Cur_page = 0;

unsigned long write_sector_offset = 0;
unsigned char font_ok = 0;
int cidx = 0;
int error_cnt[6] = {0x0,};

unsigned char buf[8192];

bool font_is_ok(void)
{
	unsigned int GB_CODE;
	
	ExitDeepSleepMode();
	
    nrf_delay_ms(1);
    
	GB_CODE = U2G(0x554a);
	
	// EntryDeepSleepMode();
	
	if ( GB_CODE == 0xb0a1 )
	{
		 return true;
	}
	return false;
}

unsigned long taskGUI( unsigned long task_id, unsigned long events )
{
	if ( events & TASK_GUI_UPDATE_EVT )
	{
		window[win_current].msg_type = WINDOW_UPDATE;
		
		osal_set_event( task_id, TASK_GUI_UI_EVT );
		
		return ( events ^ TASK_GUI_UPDATE_EVT );
	}
	
	if ( events & TASK_GUI_TOUCH_EVT )
	{
		window[win_current].msg_type |= WINDOW_TOUCH;
		
		osal_set_event( task_id, TASK_GUI_UI_EVT );

		return ( events ^ TASK_GUI_TOUCH_EVT );
	}
	
	if ( events & TASK_GUI_PRESS_EVT )
	{
		window[win_current].msg_type |= WINDOW_PRESS;
		
		osal_set_event( task_id, TASK_GUI_UI_EVT );
		
		return ( events ^ TASK_GUI_PRESS_EVT );		
	}
	
	if ( events & TASK_GUI_NOTIFY_EVT )
	{
		window[win_current].msg_type |= WINDOW_NOTIFY;
		
		osal_set_event( task_id, TASK_GUI_UI_EVT );
		
		return ( events ^ TASK_GUI_NOTIFY_EVT );
	}
	
    if ( events & TASK_GUI_UI_EVT )
    {
        unsigned long msg_type = window[win_current].msg_type;
        
        while( msg_type )
        {
            unsigned long *arg = window[win_current].arg;
            unsigned long param = window[win_current].param;
			
            msg_type = window[win_current].wproc( win_current, msg_type, param, arg ); 
			window[win_current].msg_type = msg_type;
        }
        
        return ( events ^ TASK_GUI_UI_EVT );
    }	
	
	if ( events & TASK_GUI_LCD_CONFIG_DELAY_EVT )
	{
		LCD_Config_Delay();
		
		return ( events ^ TASK_GUI_LCD_CONFIG_DELAY_EVT );
	}
	
	if ( events & TASK_GUI_INIT_EVT )
	{
		osal_set_event( task_id, TASK_GUI_UI_EVT );
	
        nrf_gpio_cfg(
            BMA_POWER_PIN,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
        nrf_gpio_pin_clear(BMA_POWER_PIN);
        
        nrf_gpio_cfg(
            BMA_SPI_CS_PIN,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
        nrf_gpio_pin_set(BMA_SPI_CS_PIN);        
        
        // 把MISO设置为输出
//        nrf_gpio_cfg(
//                FONT_SPI_MISO_PIN,
//                NRF_GPIO_PIN_DIR_OUTPUT,
//                NRF_GPIO_PIN_INPUT_DISCONNECT,
//                NRF_GPIO_PIN_PULLUP,
//                NRF_GPIO_PIN_S0S1,
//                NRF_GPIO_PIN_NOSENSE);	
               
		button_init();
		LCD_Init();
       
		font_init();
        EntryDeepSleepMode();
		
        osal_start_timerEx( task_id, TASK_GUI_FONI_INIT_EVT, 200 );
        
        // osal_set_event ( task_id, TASK_GUI_FONT_DEBUG_EVT );
        
		return ( events ^ TASK_GUI_INIT_EVT );
	}
	    
    if ( events & TASK_GUI_FONI_INIT_EVT )
    {
        erase_All_page   = 32;       // 共512页
        erase_Cur_page   = 0;
        Flash_stat       = 0;
        
        font_ok = font_is_ok();
       
        if ( font_ok == true )
        {
            osal_set_event( task_id, TASK_GUI_FONT_ERASE_SECOTOR_EVT );  
        //    osal_set_event ( task_id, TASK_GUI_FONT_READ_PAGE_EVT );
        }        

        return ( events ^ TASK_GUI_FONI_INIT_EVT );
    }
	    
    if ( events & TASK_GUI_FONT_ERASE_SECOTOR_EVT )
    {
        /**
         * 擦除0-16个用户可用扇区.每个扇区4KB，共64KB空间.
         * 
         */        
        EraseSector(FLASH_BASE_ADDR + erase_Cur_page * 4096);
        CheckStatus();
        
        if ( ++erase_Cur_page < erase_All_page )
        {
            osal_set_event ( task_id, TASK_GUI_FONT_ERASE_SECOTOR_EVT );
        }else
        {       
            write_All_page   = SIZEOF(flash_ui);       // 共256页
            write_Cur_page   = 0;
            Flash_stat       = 1;
            write_sector_offset = 0;
            osal_start_timerEx ( task_id, TASK_GUI_FONT_WRITE_PAGE_EVT, 100);
        }
        
        return ( events ^ TASK_GUI_FONT_ERASE_SECOTOR_EVT );
    }
    
    if ( events & TASK_GUI_FONT_WRITE_PAGE_EVT )
    {
        /**
         * 将片内64KB Flash空间的图片写入外部Flash中.
         *
         */
        int size = ( flash_ui[write_Cur_page].size + 0xFF ) & ~0xFF;
        
        if ( write_Cur_page < write_All_page )
        {
            if ( write_sector_offset < size/256 ) 
            {
                osal_set_event ( task_id, TASK_GUI_FONT_WRITE_PAGE_1_EVT );
            }else{
                write_sector_offset = 0;
                write_Cur_page++;
                osal_set_event ( task_id, TASK_GUI_FONT_WRITE_PAGE_EVT );
            }
        }else
        {
            osal_start_timerEx ( task_id, TASK_GUI_FONT_READ_PAGE_EVT, 200);
        }

        return ( events ^ TASK_GUI_FONT_WRITE_PAGE_EVT );
    }  

    if ( events & TASK_GUI_FONT_WRITE_PAGE_1_EVT )
    {
        /**
         * 将片内64KB Flash空间的图片写入外部Flash中.
         *
         */
        memcpy( buf, flash_ui[write_Cur_page].bitmap + write_sector_offset*256, 256 );
        
        WritePage(flash_ui[write_Cur_page].addr + write_sector_offset *256, buf, 256 );
        CheckStatus();
        
        osal_set_event ( task_id, TASK_GUI_FONT_WRITE_PAGE_EVT );
        
        write_sector_offset++;

        return ( events ^ TASK_GUI_FONT_WRITE_PAGE_1_EVT );
    }    
    
    if ( events & TASK_GUI_FONT_READ_PAGE_EVT )
    {
        /**
         * 读取外部Flash 64KB 的空间，与片内原始数据做比较, 进行数据校验.
         *
         */
        
        for ( int i = 0; i < 6; i++ )
        {
            error_cnt[i] = 0;
        }
        
        Flash_stat = 2;
        
        read_All_page   = SIZEOF(flash_ui);       // 共256页
        read_Cur_page = 0;
                
        for ( read_Cur_page = 0; read_Cur_page < read_All_page; read_Cur_page++ )
        {
            int size = flash_ui[read_Cur_page].size;
            
            memset ( buf,0,sizeof(buf) );
            
            ReadPage( flash_ui[read_Cur_page].addr, buf, size);
            
            //if ( memcmp( flash_ui[read_Cur_page].bitmap, buf, size ) != 0 )
            for ( cidx = 0; cidx < size; cidx++ )
            {
                if ( flash_ui[read_Cur_page].bitmap[cidx] != buf[cidx] )
                //break;
                error_cnt[read_Cur_page]++;
            }
        }        
        Flash_stat = 3;
        
        if ( read_Cur_page != read_All_page )
        {
            osal_start_timerEx ( task_id,TASK_GUI_FONT_READ_PAGE_EVT, 100); 
        }
        
        return ( events ^ TASK_GUI_FONT_READ_PAGE_EVT );
    } 

    if ( events & TASK_GUI_FONT_DEBUG_EVT )
    {
        static unsigned char flag = 0;
        
        osal_start_timerEx ( task_id, TASK_GUI_FONT_DEBUG_EVT, 50 );
        
        if ( flag++ & 0x1 )
        {
            nrf_gpio_pin_set(FONT_SPI_MISO_PIN);  
        }else{
            nrf_gpio_pin_clear(FONT_SPI_MISO_PIN);  
        }
        
        return ( events ^ TASK_GUI_FONT_DEBUG_EVT );
    }
    
	return ( 0 );
}

void taskGUIInit( unsigned long task_id )
{
	taskGUITaskId = task_id;
	
	window_init();	
	
    osal_start_timerEx(task_id, TASK_GUI_INIT_EVT, 200);
}

