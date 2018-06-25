#ifndef __EM70X8_H__
#define __EM70X8_H__

#include "stdint.h"

#define kal_int8	int8_t
#define kal_uint8	uint8_t
#define kal_bool  uint8_t
#define kal_uint16	uint16_t
#define kal_uint32	uint32_t
#define KAL_TRUE	1
#define KAL_FALSE	0

extern kal_uint16 data;
extern kal_uint8 bpm_data;
extern unsigned char bpm_tmp;

extern void EM70X8_blood500ms_get_data(void);
extern void EM70X8_blood50ms_get_data(void);

extern int em70xx_bpm_dynamic(int RECEIVED_BYTE, int g_sensor_x, int g_sensor_y, int g_sensor_z);
extern int em70xx_reset(int ref);
extern kal_bool HRS_WriteBytes(kal_uint8 RegAddr, kal_uint8 Data);
extern kal_bool HRS_ReadBytes(kal_uint8* Data, kal_uint8 RegAddr);

void EM7028_hrs_init(void);
void EM7028_hrs_get_data(void);
void EM7028_enable(kal_uint8 enable);
#endif

