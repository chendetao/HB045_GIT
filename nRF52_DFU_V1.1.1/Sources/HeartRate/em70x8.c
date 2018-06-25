#include <stdint.h>
#include <string.h>
#include "em70x8.h"
#include "blood.h"
#include "UserSetting.h"
#include "Debug.h"

//#define __EM7028_DEBUG__
#if defined(__EM7028_DEBUG__)
#define EM7028_TRACE  printf
#else
#define EM7028_TRACE
#endif

#define MOD_AUX		0
#define MOD_EM7028    MOD_AUX

uint16_t faraway_hand_f=0;
int em78x0_X = 0;
int em78x0_Y = 0;
int em78x0_Z = 0;

kal_uint8  bpm_data = 0x00;
kal_uint16 data = 0x00;
kal_uint16 count_hr = 0x00;
kal_uint32 num_hr = 0x00;
kal_uint8  blood_h=0;
kal_uint8  blood_l=0;
unsigned char bpm_tmp;
/*******************************************************************************
Modulm  Function
********************************************************************************/
void EM7028_hrs_get_data(void);
extern kal_uint8 GET_BP_MAX (void);
extern kal_uint8 GET_BP_MIN (void);
extern void Blood_Process (void);
extern void Get_Hr ( unsigned char data);
extern void Blood_Disable (void);
extern void Blood_50ms_process (void);
extern void Blood_500ms_process(void);
extern unsigned char leave_detec_flag;
void EM70X8_blood500ms_get_data(void);
void EM70X8_blood50ms_get_data(void);

//kal_timerid EM70X8_TimeISR_id = NULL;
//kal_timerid EM70X8_Blood50ms_id = NULL;
//kal_timerid EM70X8_Blood500ms_id = NULL;
/**寄存器地址*/
#define EM7028_PID_REG						0x00  //reservrd
#define EM7028_ENABLE_REG 		    0x01	//
#define EM7028_OFFSET_REG 		    0x08	
#define EM7028_GAIN_REG 		    	0x0A	
#define EM7028_CONFIG_REG		   	  0x0D	

//收缩压systolic pressure
//舒张压diastolic pressure

/*******************************************************************************
********************************************************************************/
kal_uint8 EM7028_hrs_pid()
{
	kal_uint8  pid = 0x00;

	HRS_ReadBytes(&pid,0x00);

	return pid;
}	

/*50ms 采集一次数据*/
void EM70X8_blood50ms_get_data(void)
{
	
		Blood_50ms_process();
		
//		if(EM70X8_Blood50ms_id == NULL)
//		EM70X8_Blood50ms_id = kal_create_timer("EM70X8_Blood50ms_id");
//	
//	kal_set_timer(EM70X8_Blood50ms_id,(kal_timer_func_ptr)EM70X8_blood50ms_get_data,NULL,6,NULL);// 5 ~=  25ms 

}
/*采集数据统计*/
void EM70X8_blood500ms_get_data(void)
{
	Blood_500ms_process();
	
//		if(EM70X8_Blood500ms_id == NULL)
//		EM70X8_Blood500ms_id = kal_create_timer("EM70X8_TimeISR");
//	
//	kal_set_timer(EM70X8_Blood500ms_id,(kal_timer_func_ptr)EM70X8_blood500ms_get_data,NULL,6,NULL);// 5 ~=  25ms 

}
void EM7028_hrs_init()
{
	HRS_WriteBytes(EM7028_ENABLE_REG,0x08);//首先启动连续测量模式
	HRS_WriteBytes(EM7028_OFFSET_REG,0x00);
	HRS_WriteBytes(EM7028_GAIN_REG,0x7f);
	HRS_WriteBytes(EM7028_CONFIG_REG,0xc7);
	HRS_WriteBytes(0x0e,0x06);
	//EM7028_hrs_pid();
	faraway_hand_f=0;
	em70xx_reset(0);

	/*Before measuring the heart rate ,you should initialize the variable to call*/
	count_hr = 0;
	num_hr=0;
}

volatile unsigned long sensor_data;

void EM7028_hrs_get_data()
{
	kal_uint8  data_l = 0x00;
	kal_uint8  data_h = 0x00;

	if(faraway_hand_f)
	{
        //Pluse Mode HRS2
		HRS_ReadBytes(&data_l,0x20);//HRS2_DATA0[7:0]
		HRS_ReadBytes(&data_h,0x21);//HRS2_DATA0[15:8]
		data = (data_h <<8) | data_l;
		if(data > 10)
		{
			HRS_WriteBytes(EM7028_ENABLE_REG,0x08);//Start Continuous Mode 
			faraway_hand_f=0;
			em70xx_reset(0);
		}
		//data=0;
	}else
    {
        //Continuous Mode HRS1 
        
        HRS_ReadBytes(&data_l,0x30);//HRS1_DATA0[7:0]
        HRS_ReadBytes(&data_h,0x31);//HRS1_DATA0[15:8]
        data = (data_h <<8) | data_l;
        if((data<10000||data>60000))
        {
            HRS_WriteBytes(EM7028_ENABLE_REG,0x80);//Start Pluse Mode To Measure Distance
            em70xx_reset(0);
            faraway_hand_f=1;
        }
        
        sensor_data = data;
    }

    // bpm_data = em70xx_bpm_dynamic(data,em78x0_X,em78x0_Y,em78x0_Z);
    
    bpm_data = em70xx_bpm_dynamic(data,0,0,0);

    Blood_Process();
    // Get_Hr(bpm_data);
    blood_h = GET_BP_MAX();
    blood_l = GET_BP_MIN();

    if ( (bpm_data > 54) && (bpm_data) < 150 )
    {
        count_hr++;
        if ( count_hr > 5 )
        {
            bpm_tmp = num_hr / 5;
            count_hr = 0;
            num_hr = 0;
        }
        else
        {
            num_hr += bpm_data;
        }
    }
}

/*关闭血压功能*/
void EM7028_enable(kal_uint8 enable)
{
	if(enable){
			HRS_WriteBytes(EM7028_ENABLE_REG,0x08);
			HRS_WriteBytes(EM7028_OFFSET_REG,0x00);
			HRS_WriteBytes(EM7028_GAIN_REG,0x7f);
			HRS_WriteBytes(EM7028_CONFIG_REG,0xc7);
			faraway_hand_f=0;
			em70xx_reset(0);
	}else{
			HRS_WriteBytes(EM7028_ENABLE_REG,0x00);
			HRS_WriteBytes(EM7028_CONFIG_REG,0x00);
			em70xx_reset(0);
			Blood_Disable();
	}

}

int SEGGER_RTT_printf(unsigned BufferIndex, const char * sFormat, ...) 
{
  return 0;
}
