/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : main.c                                                            *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : program entry point.                                            *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017-3.29  1.0        build this moudle                   *
*                                                                              *
*                                                                              *
* Created : 2017-3-29                                                          *
* Last modified : 2017.3.29                                                    *
*******************************************************************************/

#include "OSAL.h"
#include "OSAL_Timer.h"
#include "OSAL_Task.h"

#include "nrf52.h"
#include "nrf_gpio.h"

#include "QR_Utils.h"
#include "FPU.h"
#include "Debug.h"

const unsigned char hex[] = {"0123456789ABCDEF"};

void covert_int2str(unsigned char *buf, unsigned long val1, unsigned long val2)
{
	buf[0]  = hex[(val1>>8)>>4 & 0xF];
	buf[1]  = hex[(val1>>8)>>0 & 0xF];
	buf[2]  = ':';
	buf[3]  = hex[(val1>>0)>>4 & 0xF];
	buf[4]  = hex[(val1>>0)>>0 & 0xF];
	buf[5]  = ':';
	buf[6]  = hex[(val2>>24)>>4 & 0xF];
	buf[7]  = hex[(val2>>24)>>0 & 0xF];
	buf[8]  = ':';
	buf[9]  = hex[(val2>>16)>>4 & 0xF];
	buf[10] = hex[(val2>>16)>>0 & 0xF];	
	buf[11] = ':';
	buf[12] = hex[(val2>>8)>>4 & 0xF];
	buf[13] = hex[(val2>>8)>>0 & 0xF];
	buf[14] = ':';
	buf[15] = hex[(val2>>0)>>4 & 0xF];
	buf[16] = hex[(val2>>0)>>0 & 0xF];
	buf[17] = 0;
}

void gain_mac( unsigned char *buffer)
{
	unsigned long addr1 = NRF_FICR->DEVICEADDR[0];
	unsigned long addr2 = NRF_FICR->DEVICEADDR[1];

	buffer[0] =  ((addr2 & 0xFFFF) | (0xC000)) >> 8;
	buffer[1] =  (((addr2 & 0xFFFF) | (0xC000)) >> 0) & 0xFF;
	buffer[2] =  (addr1 >> 24) & 0xFF;
	buffer[3] =  (addr1 >> 16) & 0xFF;
	buffer[4] =  (addr1 >> 8) & 0xFF;
	buffer[5] =  (addr1 >> 0) & 0xFF;
}

unsigned char mac_buf[18];

int __attribute__((noreturn)) main(void)
{
 	NRF_POWER->DCDCEN = 1;  // Enable the DC/DC convertor
	
	#if (CONFIG_USING_FPU == 1)
	NVIC_SetPriority(FPU_IRQn, FPU_IRQ_PRIORITY_LOW);
	NVIC_EnableIRQ(FPU_IRQn);
    #endif
	
	/**
	 * Generate the QRCode Image by BLE Device MAC
	 */
	unsigned long addr1 = NRF_FICR->DEVICEADDR[0];
	unsigned long addr2 = NRF_FICR->DEVICEADDR[1];
	
	covert_int2str(mac_buf,  (addr2 & 0xFFFF) | (0xC000), addr1 );
	
	QR_Encode_Create(mac_buf);
	
	#if (ENABLE_DEBUG_OUTPUT == 1)
	Debug_uart_init();
	#endif
		
	osalTimerInit();
	osal_init_tasks();      // Init the OSAL Task
	osal_start_system();    // Enable the OSAL Event Scheduler
	
	for( ; ; )
	{
		 osal_run_system(); 
	}
}

