#include "SSADC.h"
#include "Board.h"

nrf_saadc_value_t  saadc_buff[SAADC_BUFF_SISE];


void saadc_init(void)
{
	nrf_saadc_channel_config_t   nrf_saadc_channel_config;
	
	nrf_saadc_disable();
	
	/* Positive channel resistor control 阻抗*/
	nrf_saadc_channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
	nrf_saadc_channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
	/* Gain factor of the analog-to-digital converter input 增益*/
	nrf_saadc_channel_config.gain       = NRF_SAADC_GAIN1_5; 
	/* Reference selection,Internal reference (0.6 V). 参考电压*/
	nrf_saadc_channel_config.reference  = NRF_SAADC_REFERENCE_INTERNAL;
	/*Analog-to-digital converter acquisition time 采集时间 */
	nrf_saadc_channel_config.acq_time   = NRF_SAADC_ACQTIME_10US;
	/*  Single ended, PSELN will be ignored, negative input to ADC shorted to GND. */
	nrf_saadc_channel_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
	/* Input selection for the analog-to-digital converter.引脚选择 */
	nrf_saadc_channel_config.pin_p      = BAT_DET_ADC_PIN;
	nrf_saadc_channel_config.pin_n      = NRF_SAADC_INPUT_DISABLED;
	nrf_saadc_channel_init(0, &nrf_saadc_channel_config);	
	
	/*14位ADC */
	nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_12BIT);
	
	
//	NRF_SAADC->SAMPLERATE = ( (160<<0)
//	                         | (0<<12)
//							);
							
	nrf_saadc_int_disable(NRF_SAADC_INT_END);
	
	/* Function for enabling the SAADC. */
	nrf_saadc_enable();
}


/* 
	RESULT = [V(P) - V(N) ] * GAIN/REFERENCE * 2(RESOLUTION - m)
	V(P)
		is the voltage at input P
	V(N)
		is the voltage at input N
	GAIN
		is the selected gain setting
	REFERENCE
		is the selected reference voltage

	and m=0 if CONFIG.MODE=SE, or m=1 if CONFIG.MODE=Diff

	2^14 = 16383
*/

nrf_saadc_value_t* saadc_buffer_convert(void)
{
	// nrf_saadc_channel_input_set(0, BAT_DET_ADC_PIN, NRF_SAADC_INPUT_DISABLED);
	
	#if 1	
	/* initializing the SAADC result buffer */
	nrf_saadc_buffer_init(saadc_buff, 1);
	
	nrf_saadc_task_trigger(NRF_SAADC_TASK_START);
    nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);
	
	uint32_t timeout = 1000000;
    while (0 == nrf_saadc_event_check(NRF_SAADC_EVENT_END) && timeout > 0)
    {
        timeout--;
    }
	#endif
	
	nrf_saadc_event_clear(NRF_SAADC_EVENT_END);
	
	return saadc_buff;
}


