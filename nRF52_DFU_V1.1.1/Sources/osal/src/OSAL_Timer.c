/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : OSAL_Timer.c                                                      *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : OSAL Timer implementation.                                      *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017-3.30  1.0        build this moudle                   *
*                                                                              *
*                                                                              *
* Created : 2017-3-30                                                          *
* Last modified : 2017.3.30                                                    *
*******************************************************************************/

#include "string.h"

#include "OSAL_Timer.h"
#include "OSAL_Task.h"
#include "OSAL.h"
#include "OSAL_ASSERT.h"

#include "ClockTick.h"

#include "nrf52.h"
#include "nrf_gpio.h"

unsigned long osal_systemClock;
  
Timer timer[OSAL_TIMER_MAX] = {0x0,};

#define osal_timer_assert(exprt) OSAL_ASSERT(exprt)
                                   
static void platform_clock_init(void);
static void platform_clock_start(void);

void osalTimerInit( void )
{
	osal_systemClock = 0;
	memset(timer, 0, sizeof(timer));
}

void osal_timer_start(void)
{
	platform_clock_init();
    platform_clock_start();
}

int osal_create_timer(unsigned long *timer_id, unsigned long mode,\
            timer_handler handler)
{
    unsigned long idx;
	/** 
	 * create a new timer
	 */
	for ( idx = 0; idx < SIZEOF(timer); idx++){
		if ( (timer[idx].event == 0 ) && 
        ( !timer[idx].handler) ){
			break;
		}
	}
    
    if ( idx < SIZEOF(timer) )
    {
        timer[idx].taskId = 0;
        timer[idx].event = 0;
        timer[idx].handler = handler;
        timer[idx].timerId = idx;
        timer[idx].run = false;
        
        *timer_id = idx;
        
        return 0;
    }
    
    return (-1);
}

int osal_start_timer(unsigned long timer_id, unsigned long timeout)
{
    unsigned long idx = timer_id;
    
    osal_timer_assert( timer_id > (SIZEOF(timer) -1 ) );
    
    timer[idx].timeout_value = timeout;
    timer[idx].run = true;
    
    return 0;
}

int osal_stop_timer(unsigned long timer_id)
{
    unsigned long idx = timer_id;

    osal_timer_assert( timer_id > (SIZEOF(timer) -1 ) );
    
    timer[idx].run = false;
    
    return 0;
}

int osal_release_timer(unsigned long timer_id )
{
    unsigned long idx = timer_id;

    osal_timer_assert( timer_id > (SIZEOF(timer) -1 ) );
    
    memset(&timer[idx], 0, sizeof(timer[idx])); 

    return 0;    
}
  
unsigned long osal_start_timerEx( unsigned long task_id, unsigned long event_id,\
			long timeout_value)
{
	unsigned long idx;
	unsigned long x = 0;
	  
	/* Entry the critical */
	 
	entry_critical(x);

	/**
	 * See if the event is exist in a task.
	 * just update the timeout value when it is exist.
	 * otherwise, we make a new.
	 */
	for ( idx = 0; idx < SIZEOF(timer); idx++){
		if ( (timer[idx].taskId == task_id) &&     
		(timer[idx].event & event_id) && 
        ( !timer[idx].handler) ){
				  
			timer[idx].timeout_value = timeout_value;
			break;
		}
	}
	  
	if ( idx < SIZEOF(timer) ){
		/* Leave the critical */
		leave_critical(x);
		return (idx >= SIZEOF(timer)) ? 0 : (task_id);
	}
	  
	/** 
	 * create a new timer
	 */
	for ( idx = 0; idx < SIZEOF(timer); idx++){
		if ( (timer[idx].event == 0 ) && 
        ( !timer[idx].handler) ){
			timer[idx].taskId = task_id;
			timer[idx].event = event_id;
			timer[idx].timeout_value = timeout_value;
            timer[idx].run = true;
			break;
		}
	}	  
	  
	/* Leave the critical */
	leave_critical(x);
	  
	return (idx >= SIZEOF(timer)) ? (0xFFFFFFFF) : (task_id);
}
				
unsigned long osal_stop_timerEx( unsigned long task_id, unsigned long event_id)
{
	unsigned long idx;
	unsigned long x = 0;
	  
	/* Entry the critical */
	 
	entry_critical(x);
	  
	for ( idx = 0; idx < SIZEOF(timer); idx++){
		if ( (timer[idx].event & event_id)
			&& (task_id == timer[idx].taskId) ){ // Bug modify
			    	  
            memset( &timer[idx], 0, sizeof(timer[idx]));
			  
			break;
		}
	}	  
	  
	/* Leave the critical */
	leave_critical(x);
	  
	return (idx >= SIZEOF(timer)) ? (0xFFFFFFFF) : (task_id);
}
    
void osal_timer_isr( unsigned long elapsedTick )
{
	unsigned long idx;
	  
	/**
  	 * Decrease all timer timeout value if the 
	 * value large than zero
	 */
	for ( idx = 0; idx < SIZEOF(timer); idx++){
        
        if ( timer[idx].run == false ) { continue; }
        
		if ( timer[idx].timeout_value > elapsedTick ){
			timer[idx].timeout_value -= elapsedTick;
		}else{
			timer[idx].timeout_value = 0;
            
            if ( timer[idx].event != 0 ){
                /* Set the event bit(s) */
                taskEventsTable[timer[idx].taskId] |= timer[idx].event; 
             	
                /* Clear the timer when it has timeout */
                memset( &timer[idx], 0, sizeof(Timer));
                               
            }else if ( timer[idx].handler )
            {
                timer[idx].run = false;
                timer[idx].handler(0);
            }else{
                osal_timer_assert(1);
            }                
		}
	}
	  
	osal_systemClock++;
}

void platform_clock_start(void)
{
    NRF_RTC1->TASKS_START = 1;// RTC   
}

#include "nrf_delay.h"

void platform_clock_stop(void)
{
    NVIC_DisableIRQ(RTC1_IRQn);

    NRF_RTC1->EVTENCLR = RTC_EVTEN_COMPARE0_Msk;
    NRF_RTC1->INTENCLR = RTC_INTENSET_COMPARE0_Msk;

    NRF_RTC1->TASKS_STOP = 1;
    nrf_delay_us(47);

    NRF_RTC1->TASKS_CLEAR = 1;
    nrf_delay_us(47);	
}

void osal_timer_disable(void)
{
	platform_clock_stop();
}

void platform_clock_init(void)
{
    // LFCLK 32.768 kHz crystal oscillator and wait it is running....
    // NRF_CLOCK->LFCLKSRC = 1;
    // NRF_CLOCK->TASKS_LFCLKSTART = 1;
    // while( (NRF_CLOCK->LFCLKSTAT&0x01) != 1);
	NVIC_DisableIRQ(RTC1_IRQn);
	NRF_RTC1->TASKS_STOP = 1;
    nrf_delay_us(47);	
	NRF_RTC1->TASKS_CLEAR = 1;
    nrf_delay_us(47);	
	NRF_RTC1->EVENTS_TICK = 0;
	NRF_RTC1->EVENTS_COMPARE[0] = 0;   
	
    // tick on 100ms
    // NRF_RTC1->PRESCALER = 3276; 
	
	// tick on 50ms
	NRF_RTC1->PRESCALER = 1638;
	// NRF_RTC1->PRESCALER = 819;
	
    // select channel compare0     
    NRF_RTC1->EVTENSET = 1<<16;
    // generate RTC interrupt when compare0 event
    NRF_RTC1->INTENSET = 1<<16;
    // NRF_RTC1->CC[0] = 10;    //1000ms	
	
	NRF_RTC1->CC[0] = 20;
	//NRF_RTC1->CC[0] = 40;
	
	// tick interrupt
	NRF_RTC1->EVTENSET = 0x1<<0;
	NRF_RTC1->INTENSET = 0x1<<0;
		
    // NVIC RTC RTC
    NVIC_SetPriority(RTC1_IRQn, 6);
    NVIC_ClearPendingIRQ(RTC1_IRQn);
    NVIC_EnableIRQ(RTC1_IRQn);  
}

unsigned long systemTick;

void RTC1_IRQHandler(void)
{	
	if ( NRF_RTC1->EVENTS_COMPARE[0] == 1 )
    {
		NRF_RTC1->EVENTS_COMPARE[0] = 0;		
		
		systemTick++;
		system_clock_click(1);	

		NRF_RTC1->TASKS_CLEAR = 1;
	} 

	if ( NRF_RTC1->EVENTS_TICK == 1 )
	{		
		NRF_RTC1->EVENTS_TICK = 0;
		
		osal_timer_isr(50);
	}	
}

/*****************************************************************
 心率相关定时器操作
 ****************************************************************/

void hr_clock_rtc_init(void)
{
    // LFCLK 32.768 kHz crystal oscillator and wait it is running....
    // NRF_CLOCK->LFCLKSRC = 1;
    // NRF_CLOCK->TASKS_LFCLKSTART = 1;
    // while( (NRF_CLOCK->LFCLKSTAT&0x01) != 1);

	NVIC_DisableIRQ(RTC2_IRQn);
	NRF_RTC2->TASKS_STOP = 1;
    nrf_delay_us(47);	
	NRF_RTC2->TASKS_CLEAR = 1;
    nrf_delay_us(47);	
	NRF_RTC2->EVENTS_TICK = 0;
	
    // tick on 100ms
    // NRF_RTC1->PRESCALER = 3276; 

	// tick on 30ms 
	NRF_RTC2->PRESCALER = 983;
	
	// tick on 20ms (50Hz)
	// NRF_RTC2->PRESCALER = 655;
	
	// tick on 80ms (12.5Hz)
	// NRF_RTC2->PRESCALER = 2621;
	
    // select channel compare0     
	// NRF_RTC2->EVTENSET = 1<<16;
    // generate RTC interrupt when compare0 event
	// NRF_RTC2->INTENSET = 1<<16;
    // NRF_RTC1->CC[0] = 10;    //1000ms	
	
	// NRF_RTC1->CC[0] = 20;
	// NRF_RTC2->CC[0] = 40;
	
	// tick interrupt
	NRF_RTC2->EVTENSET = 0x1<<0;
	NRF_RTC2->INTENSET = 0x1<<0;
		
    // NVIC RTC RTC
    NVIC_SetPriority(RTC2_IRQn, 7);
    NVIC_ClearPendingIRQ(RTC2_IRQn);
    NVIC_EnableIRQ(RTC2_IRQn);  
	
	// NRF_RTC2->TASKS_START = 1; // RTC 
}

void hr_clock_rtc_start(void)
{
    NVIC_SetPriority(RTC2_IRQn, 7);
    NVIC_ClearPendingIRQ(RTC2_IRQn);
    NVIC_EnableIRQ(RTC2_IRQn);
	
	NRF_RTC2->TASKS_START = 1;// RTC
}

void hr_clock_rtc_stop(void)
{
    NVIC_DisableIRQ(RTC2_IRQn);

    NRF_RTC2->EVTENCLR = RTC_EVTEN_COMPARE0_Msk;
    NRF_RTC2->INTENCLR = RTC_INTENSET_COMPARE0_Msk;

    NRF_RTC2->TASKS_STOP = 1;
    nrf_delay_us(47);

    NRF_RTC2->TASKS_CLEAR = 1;
    nrf_delay_us(47);
}

volatile unsigned long hr_mticks;
extern void hr_callback(void);

void RTC2_IRQHandler(void)
{	
	if ( NRF_RTC2->EVENTS_TICK == 1 )
	{		
		NRF_RTC2->EVENTS_TICK = 0;
		
        hr_mticks += 30;
		
        hr_callback();
	}	
}

/*****************************************************************
				The End
 ****************************************************************/
