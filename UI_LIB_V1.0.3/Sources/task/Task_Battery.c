#include "string.h"
#include "OSAL.h"
#include "OSAL_Timer.h"

#include "Task_Battery.h"

#include "btprotocol.h"
#include "ClockTick.h"
#include "SSADC.h"
#include "Board.h"
#include "nrf_gpio.h"
#include "UserSetting.h"
#include "UI_win_sport.h"
#include "Window.h"

Power_Typdef power = 
{
	.battery_level 		= 0,
	.battery_low   		= 0,
	.charge_flage  		= 0,
	.ble_signal    		= 0,
	.compelete 	   		= 0,
	.time_after_last 	= 0,
	.debug = 0,
};

#include "LookTable.h"

unsigned long taskBatteryTaskId;
short current_adc;

#define POWER()  nrf_gpio_pin_read(BAT_CHARGE_PIN)
#define CHARE()  nrf_gpio_pin_read(BAT_STAT_DET_PIN)
#define LED_ALL_OFF()
#define LED_GREEN_ON()
#define LED_RED_ON()

unsigned char last_battery_level;
unsigned char battery_flag = 0;

unsigned long taskBattery( unsigned long task_id, unsigned long events )
{	
	return 0;
}

void taskBatteryInit( unsigned long task_id )
{
	taskBatteryTaskId = task_id;	

	nrf_gpio_pin_pull_t config1 = NRF_GPIO_PIN_PULLUP;
	nrf_gpio_cfg_input(BAT_STAT_DET_PIN, config1);

	/** GPIOTE0 chanenl 1 as event
	 *  for GPIO pin BAT_STAT_DET_PIN (3)
	 *  when any change on pin interrupt
	 */
	NRF_GPIOTE->CONFIG[2] = (1<<0) |
					 (BAT_STAT_DET_PIN << 8) |
					 (3 << 16);
	// Enable interrupt
	NRF_GPIOTE->INTENSET |= 0x1UL<<2;
	
	
	nrf_gpio_pin_pull_t config2 = NRF_GPIO_PIN_NOPULL;
	nrf_gpio_cfg_input(BAT_CHARGE_PIN, config2);

	/** GPIOTE0 chanenl 1 as event
	 * for GPIO pin BAT_CHARGE_PIN (6)
	 *  when any change on pin trigger interrupt
	 */
	NRF_GPIOTE->CONFIG[3] = (1<<0) |
					 (BAT_CHARGE_PIN << 8) |
					 (3 << 16);
	// Enable interrupt
	NRF_GPIOTE->INTENSET |= 0x1UL<<3;
	
	battery_flag = 0;
	power.ok = 1;
		
	config.motor_test_enable = 1;	
		
	osal_set_event ( task_id, TASK_BATTERY_INIT_EVT );
}

