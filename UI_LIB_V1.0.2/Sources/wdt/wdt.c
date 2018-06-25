
#include "nrf52.h"
#include "nrf_wdt.h"
#include "wdt.h"
/**
 * @brief WDT events handler.
 */

void WDT_Init(void)
{	
	NRF_WDT->CONFIG = NRF_WDT_BEHAVIOUR_RUN_SLEEP_HALT;
	NRF_WDT->CRV = (2000* 32768) / 1000;
	NRF_WDT->RREN |= 0x1UL << 0;
	NRF_WDT->TASKS_START = 1;
}


void WDT_Feed(void)
{
	NRF_WDT->RR[0] = 0x6E524635UL;	
}
