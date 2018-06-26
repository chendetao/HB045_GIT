#ifndef __TASK_HR_H__
#define __TASK_HR_H__

#define TASK_HR_SAMPLE_EVT 				0x00000001
#define TASK_HR_STOP_EVT   				0x00000002
#define TASK_HR_START_EVT  				0x00000004
#define TASK_HR_DECT_DONE_HR_EVT    	0x00000008
#define TASK_HR_DECT_DONE_HR_UPLOAD_EVT 0x00000010

#define TASK_HR_FFT_ENABLE_EVT      	0x00000020

#define TASK_HR_LEAVE_EVT           	0x00000040
#define TASK_HR_NORMAL_EVT          	0x00000080
#define TASK_HR_DECT_AUTO_EVT       	0x00000100

#define TASK_HR_SENSOR_OK_EVT       	0x00000200
#define TASK_HR_PW_ENABLE_EVT           0x00000400
#define TASK_HR_SENSOR_COMFIR_EVTT      0x00000800

#if 1
#define HR_ADC_PIN_OUTPUT_LOW() do\
{\
	nrf_gpio_cfg_output(HR_ADC_PIN);\
	nrf_gpio_pin_clear(HR_ADC_PIN);\
}while(0)
#else
#define HR_ADC_PIN_OUTPUT_LOW()
#endif

extern volatile unsigned char hr_showFlag;
extern int to_close;
extern volatile unsigned char last_hr;
extern volatile unsigned long need_mTicks;
extern short current_touch_adc;

extern unsigned long taskHrTaskId;
extern unsigned long taskHr( unsigned long task_id, unsigned long events );
extern void taskHrInit( unsigned long task_id );

#endif
