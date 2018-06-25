/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved                   *
*                                                                              *
* Filename : OSAL_TaskDemo.c                                                   *
* Author : Wen Shifang                                                         *
* Version : 1.0                                                                *
*                                                                              *
* Decription : A simple Task instance for OSAL                                 *
* Function-list:                                                               *
* History:                                                                     *
*       <author>     <time>     <version>  <desc>                              *
*       Wen Shifang  2017-3.30  1.0        build this moudle                   *
*                                                                              *
*                                                                              *
* Created : 2017-3-30                                                          *
* Last modified : 2017.3.30                                                    *
*******************************************************************************/

#include "Task_UItraviolet.h"
#include "OSAL.h"
#include "OSAL_Task.h"
#include "OSAL_Timer.h"

#include "SSADC.h"
#include "UI_win_notify.h"
#include "ClockTick.h"
#include "Task_Store.h"
#include "Task_Flash.h"

static nrf_saadc_value_t  saadc_buff[SAADC_BUFF_SISE];


static void saadc_init__(void)
{
	nrf_saadc_channel_config_t   nrf_saadc_channel_config;
	
	nrf_saadc_disable();
	
	/* Positive channel resistor control 阻抗*/
	nrf_saadc_channel_config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;
	nrf_saadc_channel_config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;
	/* Gain factor of the analog-to-digital converter input 增益*/
	nrf_saadc_channel_config.gain       = NRF_SAADC_GAIN1_2; 
	/* Reference selection,Internal reference (0.6 V). 参考电压*/
	nrf_saadc_channel_config.reference  = NRF_SAADC_REFERENCE_INTERNAL;
	/*Analog-to-digital converter acquisition time 采集时间 */
	nrf_saadc_channel_config.acq_time   = NRF_SAADC_ACQTIME_20US;
	/*  Single ended, PSELN will be ignored, negative input to ADC shorted to GND. */
	nrf_saadc_channel_config.mode       = NRF_SAADC_MODE_SINGLE_ENDED;
	/* Input selection for the analog-to-digital converter.引脚选择 */
	nrf_saadc_channel_config.pin_p      = UITR_ADC_PIN;
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

static nrf_saadc_value_t* saadc_buffer_convert__(void)
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

unsigned long taskUItravioletTaskId;

volatile short UIT_adc = 0;
float UIT_vol;

float UIT_cm2[2];
float UIT_i[2];

#include <math.h>
#include "btprotocol.h"


// 电压转 mw/cm2

#define       p1        -1538  
#define       p2         2690  
#define       p3        273.2   
#define       p4        -3507  
#define       p5         2952   
#define       p6        -1050   
#define       p7        168.1   
#define       p8       -3.305   
#define       p9       0.3219   

float v_to_cm2 ( float vol )
{

    float cm2 = p1 * pow(vol,8) + p2*pow(vol,7) + p3*pow(vol,6) + p4*pow(vol,5) + 
                    p5*pow(vol,4) + p6*pow(vol,3) + p7*pow(vol,2) + p8*vol + p9;
    
    return cm2;
}

// mw/cm2 转 紫外线等级
#define       p11     -0.00359  
#define       p12      0.08089  
#define       p13       -0.747   
#define       p14        3.647   
#define       p15       -10.15   
#define       p16        16.29   
#define       p17       -14.25   
#define       p18        6.701   
#define       p19       -1.118

float cm_to_i( float cm2 )
{
    float i = p11*pow(cm2,8) + p12*pow(cm2,7) + p13*pow(cm2,6) + p14*pow(cm2,5) + 
                    p15*pow(cm2,4) +
    p16*pow(cm2,3) +
    p17*pow(cm2,2) + 
    p18*pow(cm2,1) + p19;
    
    return i;
}

int UIT_adcs[3] = {0};
int dangerius_cnt = 0;
#define MAX_UIT     2
unsigned char uit_notify_enable;
unsigned char dataCnt = 0;
unsigned long sampleCnt = 0;

unsigned char local_tx[20];

unsigned long taskUItraviolet( unsigned long task_id, unsigned long events )
{
	if ( events  &  TASK_UITRAVIOLET_START_EVT )
	{      
        short * p_adc_value;
            
        saadc_init__();
        
		p_adc_value = saadc_buffer_convert__();
		if ( (short)(p_adc_value[0]) < 0 ){
			p_adc_value[0] = 0;
		}
		if ( (short)(p_adc_value[0]) > 4095 )
		{
			p_adc_value[0] = 4095;
		}        
        
        UIT_adc = p_adc_value[0];
        UIT_adcs[0] = UIT_adcs[1];
        UIT_adcs[1] = UIT_adcs[2];
        UIT_adcs[2] = UIT_adc;
        
        UIT_adc = (UIT_adcs[0]+UIT_adcs[1]+UIT_adcs[2])/3;
        
        UIT_vol = UIT_adc * (0.6f/4096.0f) * 2;
        
        UIT_cm2[0] = v_to_cm2(UIT_vol);
        
        #if 1
        UIT_i[0] = cm_to_i(UIT_cm2[0]);
        #else
        
        UIT_i[0] += 1;
        if ( UIT_i[0] > 11 )
        {
            UIT_i[0] = 0;
        }
        #endif
        
        if ( UIT_i[0] > MAX_UIT )
        {
            dangerius_cnt++;
            
            if ( dangerius_cnt > 30000/200 )
            {
                dangerius_cnt = 0;

                NOTICE_ADD( NOTIFY_ID_UIT );
            }
        }else{
            dangerius_cnt = 0;
        }            
        
        sampleCnt++;

        osal_start_timerEx ( task_id, TASK_UITRAVIOLET_START_EVT, 1000 );
        
        if ( (uit_notify_enable & 0x1 ) && !( sampleCnt % 5) ) /** 3秒上传一次 */
        {
            osal_set_event ( task_id, TASK_UIT_UPDATE_UV_EVT );
        }
        
        if ( !(sampleCnt % 30) )           /* 3分钟保存1个,1次保存2个,6分钟1组 */
        {
            if ( dataCnt == 0 )
            {
                UIT_i[1]   = UIT_i[0];
                UIT_cm2[1] = UIT_cm2[0]; 
                dataCnt = 1;
            }else{
                dataCnt = 0;
                osal_set_event ( taskStoreTaskId, TASK_STORE_SAVE_UVT_EVT ); 
            } 
        }
        
        if ( !(sampleCnt % 60) )         /* 每个60分钟上报一次 */
        {
            local_tx[0] = 0x28;
            local_tx[1] = 0x02;
            local_tx[2] = fm.erea[FMC_ID_UIT].items & 0xFF;     

            bt_protocol_tx( local_tx, sizeof(local_tx));            
        }
        
		return ( events ^ TASK_UITRAVIOLET_START_EVT );
	}
    
    if ( events & TASK_UIT_UPDATE_UV_EVT )
    {
        unsigned char *p = (unsigned char *)&UIT_cm2;
        
        local_tx[0] = 0x28;
        local_tx[1] = 0x00;
        local_tx[2] = (int)UIT_i[0];
        local_tx[3] = p[3];
        local_tx[4] = p[2];
        local_tx[5] = p[1];
        local_tx[6] = p[0];
        
        bt_protocol_tx( local_tx, sizeof(local_tx));
        
        return ( events ^ TASK_UIT_UPDATE_UV_EVT );
    }
	
	if ( events & TASK_UITRAVIOLET_STOP_EVT )
	{
		return ( events ^ TASK_UITRAVIOLET_STOP_EVT );
	}
    
    if ( events & TASK_UITRAVIOLET_INIT_EVET )
    {
        osal_set_event ( task_id, TASK_UITRAVIOLET_START_EVT );
        
        return ( events ^ TASK_UITRAVIOLET_INIT_EVET );
    }
	
	return ( 0 );
}

void taskUItravioletInit( unsigned long task_id )
{
	taskUItravioletTaskId = task_id;
	
    osal_set_event( task_id, TASK_UITRAVIOLET_INIT_EVET );
}

