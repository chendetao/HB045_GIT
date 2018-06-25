#include "HeartComp.h"

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
 * 1, 从Sensor获取心率值，作为基础心率值( 无传感器时，根据用户年龄、身高、体重等
         信息，计算近基础心率。)
 * 2, 根据最近一段时间的运动量进行补偿,运动量由GSensor的数据进行计算。
 *    (1) 最近4分钟每分钟的步数，步频等，分段补偿(参考用户信息)。
 *    (2) 根据最近4分钟内，GSensor XYZ的变化幅度来计算补偿系数,(0.0-0.9)
 *    
 */
static unsigned long comp_array_pedometer[COM_ARRAY_SIZE],last_pedometer;
static unsigned char maxHeart, baseHeart;
static unsigned char bl_pressure, bh_pressure;
static unsigned char rand = 0;
static const unsigned char randTable[16]= {3,10,2,11,3,10,0,10,13,4,11,4,12,1,13,6};
static const unsigned char randTable2[16]= {3,7,8,2,9,1,8,0,10,2,11,3,12,10,13,6};
/**
 *  void heart_comp_pedometer( uint32 val );
 *  @param val 当前的步伐数.
 *  @return none
 *  该函数需要每10秒调用一次.
 */
void heart_comp_pedometer( unsigned long val )
{
   unsigned char i, hr_baseline;
   unsigned long sum_pedometer = 0;
   
   for ( i = 0; i < COM_ARRAY_SIZE-1; i++ )
   {
      comp_array_pedometer[i] = comp_array_pedometer[i+1];
   }

   comp_array_pedometer[i] = val - last_pedometer;
   
   last_pedometer = val;
   
   /**
    * 计算4分钟内步伐总的增量值.
    */     
   for ( unsigned int i = 0; i < COM_ARRAY_SIZE; i++)
   {
       sum_pedometer += comp_array_pedometer[i];
   }
    
   hr_baseline = sum_pedometer / COM_ARRAY_SIZE;   
   
   if ( hr_baseline < val )
   {
     /**
      * 
      * 步数有下降趋势：
      * 所有值减1，避免数组没有填充满时，步伐增量一直为零时补偿因子恒定
      */
     for ( unsigned int  i = 0; i < COM_ARRAY_SIZE; i++)
     {
         if( comp_array_pedometer[i] > 1 )
         {
            comp_array_pedometer[i] -= 1;
         }
     }   
  }
   
   rand++;
}

/**
 *  void heart_comp_gsensor( int16 x, int16 y, int16 z);
 *  @param x Gsensor X轴
 *  @param y Gsensor Y轴
 *  @param z Gsensor Z轴
 *  @return none
 *  该函数需要10秒调用一次.
 */
void heart_comp_gsensor( short x, short y, short z)
{
  
}


/**
 * uint8 heart_get_comp_val(void);
 * @func 获取补偿后的心率值.
 * @val 基础心率值.
 * @return none
 */
unsigned char heart_get_comp_val(void)
{
    unsigned long sum_pedometer = 0;
    unsigned char comp_hr;
    
    /**
     * 计算4分钟内步伐总的增量值.
     */    
    for ( int i = 0; i < COM_ARRAY_SIZE; i++)
    {
       sum_pedometer += comp_array_pedometer[i];
    }
    
    /**
     * 计算步伐增量占人类理论最大步伐数的百分比，
     * 该值factor作为初次补偿系数.
     */
    float factor = (float)sum_pedometer/MAX_PEDOMETER_AT_MINUTES;
    
    /**
     * 第一次补偿: 根据步伐补偿.
     */
    comp_hr = (unsigned char)( baseHeart + (maxHeart-baseHeart) * factor);

    /**
     * 第二次补偿: 根据GSensor 3轴变化幅度 进行补偿.
     */
 
    /**
     * 第三次补偿: 年龄，身高，体重 进行补偿.
     */    
    
    /**
     * 第四次补偿：增加随机波动值.
     *
     * 基本处于静止状态,波动值限制在0-6
     * 基本处于活动状态,波动值限制在0-12
     * 基本非常活跃状态,波动值限制在0-20
     */
    if ( comp_hr < 128 )
    {
      if ( factor < (float)0.00001 )
      {
        comp_hr += randTable[rand]%7; 
      }else if ( factor < (float)0.10000 ){     
        comp_hr += randTable[rand]%13; 
      }else{
        comp_hr += randTable[rand]%21; 
      }
    }else
    {
      comp_hr = 128 + randTable2[rand]%11; 
    }
    
    rand++;
    rand %= sizeof(randTable);
    
    return comp_hr;
}

/**
 * uint8 heart_get_comp_blood_pressure_val(void);
 * @func 获取补偿后的血压.
 * @val 基础心率值.
 * @return none
 */
unsigned short heart_get_comp_blood_pressure_val(int hr)
{
    unsigned long sum_pedometer = 0;
    unsigned char comp_pressure_h,comp_pressure_l;
    
    /**
     * 计算4分钟内步伐总的增量值.
     */    
    for ( int  i = 0; i < COM_ARRAY_SIZE; i++)
    {
       sum_pedometer += comp_array_pedometer[i];
    }
    
    /**
     * 计算步伐增量占人类理论最大步伐数的百分比，
     * 该值factor作为初次补偿系数.
     */
    float factor = (float)sum_pedometer/MAX_PEDOMETER_AT_MINUTES;
    
    /**
     * 第一次补偿: 根据步伐补偿.
     */
    comp_pressure_h = (unsigned char)( bh_pressure + (bl_pressure*0.1) * factor);
    comp_pressure_l = (unsigned char)( bl_pressure + (bl_pressure*0.1) * factor);
    
	
	/**
	 * 第二次：根据心率值补偿
	 */
	comp_pressure_h = comp_pressure_h + hr*0.08 + (hr %12);
	comp_pressure_l = comp_pressure_l + hr*0.08 + (hr %8);
	
    /**
     * 第三次补偿：增加随机波动值.
     *
     * 基本处于静止状态,波动值限制在0-3
     * 基本处于活动状态,波动值限制在0-5
     * 基本非常活跃状态,波动值限制在0-8
     */
    if ( factor < (float)0.00001 )
    {
      comp_pressure_h += randTable[rand]%4;
    }else if ( factor < (float)0.10000 ){     
      comp_pressure_h += randTable[rand]%8;
    }else{
      comp_pressure_h += randTable[rand]%12;
    }
    
    rand++;
    rand %= sizeof(randTable);
       
    if ( factor < (float)0.00001 )
    {
      comp_pressure_l += randTable[rand]%4;
    }else if ( factor < (float)0.10000 ){     
      comp_pressure_l += randTable[rand]%7;
    }else{
      comp_pressure_l += randTable[rand]%10;
    }    
    
    rand++;
    rand %= sizeof(randTable);    
    
    return (comp_pressure_h<<8) | comp_pressure_l;
}
 
float heart_get_comp_spo2( int hr)
{
	int val = hr * 0.015f;
	
	return val;
}
 
/**
 * void heart_set_base_heart( uint8 val );
 * @func 设置基本心率,作为补偿基数.该值可由心率传感器传入或者由用户信息计算出.
 * @val 基础心率值.
 * @return none
 */
void heart_set_base_heart( unsigned char val )
{
   baseHeart = val;
}

/**
 * void heart_comp_init(uint8 year, uint8 height, uint8 weight );
 * @func 算法参数初始化.
 * @year 年龄
 * @height 身高
 * @weight 体重
 * @return none
 */
void heart_comp_init(unsigned char year, unsigned char height,
	unsigned char weight )
{
   for ( int i = 0; i <COM_ARRAY_SIZE; i++ )
   {
      comp_array_pedometer[i] = 0;
   }
     
   maxHeart = 220 - year;    /* 最大心率值 */
   
   /* 基本心率值 */
   last_pedometer = 0;
   heart_set_base_heart( 65 + (10 - ((unsigned char)(year/120.0*10))) );
   
   bl_pressure = 70;
   bh_pressure = 110 + 8 * (unsigned char)(( weight*0.5 + height*0.5)/510);
}
