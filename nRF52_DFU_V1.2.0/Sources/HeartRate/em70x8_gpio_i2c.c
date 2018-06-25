#if defined(SENSOR_EM7028)

#include "nrf_gpio.h"

#include "Board.h"
#include "nrf_delay.h"
#include "em70x8.h"

/*
*IIC数字脚，时钟脚
*/

/*
*设备地址
*/
#define HRS_DEVICE_WRITE_ADDRESS	 	0x48
#define HRS_DEVICE_READ_ADDRESS 		0x49

/******************************************************************************
**
**	I2c Defines
**
*******************************************************************************/
 
#define     HRS_CLK_PIN_GPIO_MODE		nrf_gpio_cfg_output(HRS_SCL_PIN)
#define     HRS_DATA_PIN_GPIO_MODE		nrf_gpio_cfg_output(HRS_SDA_PIN)

#define     HRS_I2C_CLK_OUTPUT			nrf_gpio_cfg_output(HRS_SCL_PIN)
#define     HRS_I2C_DATA_OUTPUT			nrf_gpio_cfg_output(HRS_SDA_PIN)
#define     HRS_I2C_DATA_INPUT		   	nrf_gpio_cfg_input(HRS_SDA_PIN,NRF_GPIO_PIN_PULLUP)

#define		HRS_I2C_CLK_HIGH			nrf_gpio_pin_set(HRS_SCL_PIN)
#define     HRS_I2C_CLK_LOW			  	nrf_gpio_pin_clear(HRS_SCL_PIN)
#define		HRS_I2C_DATA_HIGH			nrf_gpio_pin_set(HRS_SDA_PIN)
#define 	HRS_I2C_DATA_LOW		    nrf_gpio_pin_clear(HRS_SDA_PIN)

#define	    HRS_I2C_GET_BIT				nrf_gpio_pin_read(HRS_SDA_PIN)

/******************************************************************************
**
**	I2c Platform Functions
**
*******************************************************************************/

static void HRS_i2c_udelay(unsigned int time)
{
	nrf_delay_us(time);
}

/*
*当CLK是高电平的时候，DATA由高变低表示I2C起始
*/
static void HRS_i2c_begin(void)
{
	HRS_CLK_PIN_GPIO_MODE;
	HRS_I2C_CLK_OUTPUT;

	HRS_DATA_PIN_GPIO_MODE;
	HRS_I2C_DATA_OUTPUT;
	
	HRS_i2c_udelay(5);//(20);//(40); 	//20
	
	HRS_I2C_DATA_HIGH;
	HRS_I2C_CLK_HIGH;
	HRS_i2c_udelay(2);//(20);//(40); 	//20
	HRS_I2C_DATA_LOW;
	HRS_i2c_udelay(2);//(20);		//10
	HRS_I2C_CLK_LOW;
	HRS_i2c_udelay(20);//(20);		//10
}

/*
*当CLK是高电平的时候，DATA由低变高表示I2C结束
*/

static void HRS_i2c_end(void)
{
	HRS_I2C_CLK_OUTPUT;
	HRS_I2C_DATA_OUTPUT;
	
	HRS_i2c_udelay(10);		//10
	HRS_I2C_CLK_HIGH;
	HRS_i2c_udelay(20);		//10
	HRS_I2C_DATA_HIGH;
	//HRS_i2c_udelay(10);		//10
	//HRS_i2c_udelay(10);		//10
}
void HRS_i2c_one_clk(void)
{
	HRS_i2c_udelay(10);//(20);		//5
	HRS_I2C_CLK_HIGH;
	HRS_i2c_udelay(2);//(40);		//10
	HRS_I2C_CLK_LOW;
	HRS_i2c_udelay(2);//(20);		//5
}

/******************************************
	software I2C read byte with ack
*******************************************/
kal_uint8 HRS_ReadByteAck(void)
{
	kal_int8 i;
	kal_uint8 data;

	HRS_I2C_DATA_INPUT; 
	data = 0; 
	
	for (i=7; i>=0; i--) 
	{
		if (HRS_I2C_GET_BIT)
		{
			data |= (0x01<<i);
		}
		HRS_i2c_one_clk();
	}			                                

	/**send  ack :  data pin set low*/
	HRS_I2C_DATA_OUTPUT;                    
	HRS_I2C_DATA_LOW;                       
	HRS_i2c_one_clk();                         

	return data;
}

/******************************************
	software I2C read byte without ack
*******************************************/
kal_uint8 HRS_ReadByteNAck(void)
{
	kal_int8 i;
	kal_uint8 data;

	HRS_I2C_DATA_INPUT; 
	data = 0; 
	
	for (i=7; i>=0; i--) 
	{
		if (HRS_I2C_GET_BIT)
		{
			data |= (0x01<<i);
		}
		HRS_i2c_one_clk();
	}			                                

	/*not send ack data pin set high*/
	HRS_I2C_DATA_OUTPUT;                                           
	HRS_I2C_DATA_HIGH;
	HRS_i2c_one_clk();                         
	
	return data;
}

void HRS_SendByte(kal_uint8 sData) 
{
	kal_int8 i;
	
	for (i=7; i>=0; i--) 
	{            
		if ((sData>>i)&0x01) 
		{               
			HRS_I2C_DATA_HIGH;	              
		}
		else 
		{ 
			HRS_I2C_DATA_LOW;
		}
		HRS_i2c_one_clk();                        
	}		
}

static kal_bool HRS_Chkack(void)
{
	kal_bool result = KAL_FALSE;

	HRS_I2C_DATA_INPUT;
	HRS_i2c_udelay(2);		//5
	HRS_I2C_CLK_HIGH;
	HRS_i2c_udelay(2);		//5

	if(HRS_I2C_GET_BIT)		//Non-ack
	{
		HRS_i2c_udelay(10);	//5
		HRS_I2C_CLK_LOW;
		HRS_i2c_udelay(10);	//5
		HRS_I2C_DATA_OUTPUT;
		HRS_I2C_DATA_LOW;
		
		result = KAL_FALSE;
	}
	else					//Ack
	{
		HRS_i2c_udelay(10);	//5
		HRS_I2C_CLK_LOW;
		HRS_i2c_udelay(10);	//5
		HRS_I2C_DATA_OUTPUT;
		HRS_I2C_DATA_LOW;

		result =  KAL_TRUE;
	}

	
	return result;
	
}
/******************************************
	software I2C restart bit
*******************************************/

void HRS_Restart(void)
{
	HRS_I2C_CLK_OUTPUT;
	HRS_I2C_DATA_OUTPUT;

	HRS_i2c_udelay(40);
	HRS_I2C_DATA_HIGH;
	HRS_i2c_udelay(20);		//10
	HRS_I2C_CLK_HIGH;
	HRS_i2c_udelay(40);
	HRS_I2C_DATA_LOW;
	HRS_i2c_udelay(20);		//10
	HRS_I2C_CLK_LOW;
	HRS_i2c_udelay(20);		//10
}
void HRS_DelayMS(kal_uint16 delay)
{
	kal_uint16 i=0;

	for(i=0; i<delay; i++)
	{
		HRS_i2c_udelay(1000);
	}
}
/******************************************
	 read bytes
*******************************************/
kal_bool HRS_ReadBytes(kal_uint8* Data, kal_uint8 RegAddr)
{
	HRS_i2c_begin();						//start bit
	HRS_SendByte(HRS_DEVICE_WRITE_ADDRESS);		//slave address|write bit
	if(KAL_FALSE == HRS_Chkack())		//check Ack bit
	{
		
		HRS_i2c_end();
		return KAL_FALSE;
	}
		
	HRS_SendByte(RegAddr);				//send RegAddr
	if(KAL_FALSE == HRS_Chkack())		//check Ack bit
	{
		
		HRS_i2c_end();
		return KAL_FALSE;
	}

	HRS_Restart();						//restart bit

	HRS_SendByte(HRS_DEVICE_READ_ADDRESS);		//slave address|read bit
	if(KAL_FALSE == HRS_Chkack())
	{
		
		HRS_i2c_end();
		return KAL_FALSE;
	}

	*Data = HRS_ReadByteNAck();
	
	HRS_i2c_end();						//stop bit
	return KAL_TRUE;
	
}
/******************************************
	 write bytes
*******************************************/
kal_bool HRS_WriteBytes(kal_uint8 RegAddr, kal_uint8 Data)
{
	HRS_i2c_begin();						//start bit

	HRS_SendByte(HRS_DEVICE_WRITE_ADDRESS);		//slave address|write bit
	if(KAL_FALSE == HRS_Chkack())		//check Ack bit
	{
		
		HRS_i2c_end();
		return KAL_FALSE;
	}

	HRS_SendByte(RegAddr);				//send RegAddr
	if(KAL_FALSE == HRS_Chkack())		//check Ack bit
	{
		
		HRS_i2c_end();
		return KAL_FALSE;
	}

	HRS_SendByte(Data);					//send parameter
	if(KAL_FALSE == HRS_Chkack())
	{
		
		HRS_i2c_end();
		return KAL_FALSE;
	}

	HRS_i2c_end();						//stop bit

	return KAL_TRUE;
}



#endif
