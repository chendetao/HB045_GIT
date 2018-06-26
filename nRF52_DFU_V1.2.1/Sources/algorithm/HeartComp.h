/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved 
*
* Filename : HeartComp.h
* Author : Wen Shifang
* Version : 1.0
*
* Decription : File HeartComp.c implements the heart compensate function.
* Function-list:
* History:
*       <author>     <time>    <version>  <desc>
*       Wen Shifang  2017-3.2  1.0        build this moudle
*
* Created : 2017-3.2 
* Last modified : 2017-3.2 
*******************************************************************************/


/**
 * 动态心率补偿算法:
 * 1, 从Sensor获取心率值，作为补偿基数( 无传感器时，根据用户年龄、身高、体重等
         信息，计算近视基数。)
 * 2, 根据最近一段时间的运动量进行补偿,运动量由GSensor的数据进行计算。
 *    (1) 最近4分钟每分钟的步数，步频等，分段补偿(参考用户信息)。
 *    (2) 根据最近4分钟内，GSensor XYZ的变化幅度来计算补偿系数,(0.0-0.9)
 *    
 */
#ifndef __HEART_COMP_H__
#define __HEART_COMP_H__
   
#define MINUTE                   2     /* 最近4分钟 */   
#define UPDATE_RATE              15    /* 15秒更新一次 */
   
#define COM_ARRAY_SIZE          ((60*MINUTE)/UPDATE_RATE)
#define MAX_PEDOMETER_AT_MINUTES ((1000/250)*60*MINUTE)

extern void heart_comp_pedometer( unsigned long val );
extern void heart_comp_gsensor( short x, short y, short z);
extern unsigned char heart_get_comp_val(void);
extern unsigned short heart_get_comp_blood_pressure_val(int hr);
extern void heart_set_base_heart( unsigned char val );
extern void heart_comp_init(unsigned char year, unsigned char height,
	unsigned char weight );
extern float heart_get_comp_spo2( int hr);

#endif
