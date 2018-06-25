/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : OSAL_Lib.c                                                        *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : Power Manger Module for OSAL.                                   *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017-3.29  1.0        build this moudle                   *
*                                                                              *
*                                                                              *
* Created : 2017-3-29                                                          *
* Last modified : 2017.3.29                                                    *
*******************************************************************************/
#include "nrf52.h"

#include "OSAL_PwrMgr.h"
#include "OSAL.h"
#include "OSAL_Timer.h"
#include "FPU.h"

extern void power_manage(void);

/**
 * Power conserve function implementations.
 * This function depends on the specific hardware platform.
 */
void osal_pwrmgr_powerconserve( void )
{
	#if (CONFIG_USING_FPU == 1)
	set_FPSCR(get_FPSCR()  & ~(FPU_EXCEPTION_MASK));      
    (void) get_FPSCR();
    NVIC_ClearPendingIRQ(FPU_IRQn);
	#endif
    power_manage();
}

