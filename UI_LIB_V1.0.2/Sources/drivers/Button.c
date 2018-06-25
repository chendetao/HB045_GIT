
#include "nrf_gpio.h"
#include "Button.h"

#include "OSAL.h"
#include "Task_GUI.h"

#include "Board.h"

//#define BUTTON_PWR_ON() nrf_gpio_pin_clear(BUTTON_PWR_PIN)
//#define BUTTON_PWR_OFF() nrf_gpio_pin_set(BUTTON_PWR_PIN)

//void asc0111b_power_init(void)
//{
//	nrf_gpio_cfg(
//            BUTTON_PWR_PIN,
//            NRF_GPIO_PIN_DIR_OUTPUT,
//            NRF_GPIO_PIN_INPUT_DISCONNECT,
//            NRF_GPIO_PIN_PULLDOWN,
//            NRF_GPIO_PIN_S0S1,
//            NRF_GPIO_PIN_NOSENSE);
//	
//	BUTTON_PWR_ON();
//}

void button_init(void)
{
	// asc0111b_power_init();
	
	nrf_gpio_pin_pull_t config = NRF_GPIO_PIN_PULLUP;
    nrf_gpio_cfg_input(BUTTON_PIN, config);

    /** GPIOTE0 chanenl 1 as event
	 * for GPIO pin BUTTON_PIN (4)
	 * fail trigger interrupt
	 */
    NRF_GPIOTE->CONFIG[1] = (1<<0) |
                     (BUTTON_PIN << 8) |
                     (2 << 16);
	// Enable interrupt
	NRF_GPIOTE->INTENSET |= 0x1UL<<1;
	  
	// Set interrupt priority 
    NVIC_SetPriority(GPIOTE_IRQn, 1);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_EnableIRQ(GPIOTE_IRQn);
}

// Note GPIOTE_IRQHandler Defined at IRQHandler.c : GPIOTE_Handler
