#include "LCD_ST7735.h"
#include "nrf_delay.h"
#include "UserSetting.h"
#include "Task_Step.h"
#include "OSAL_Timer.h"
#include "Task_GUI.h"
#include "OSAL.h"


#define NEW_LCD_USED 0

unsigned short frame_buffer[160][80];
unsigned char lcd_enable_flag = 0;

void LCD_Config(void);

void LCD_SPI_Init(void)
{
	//1.CS
	nrf_gpio_cfg(
            LCD_SPI_CS,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
	//д˖ӡҪȳʨ׃
	nrf_gpio_pin_set(LCD_SPI_CS);
	
	//2.SDA
	nrf_gpio_cfg(
            LCD_SPI_SDA,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_pin_set(LCD_SPI_SDA);
	
	//3.SCL
	nrf_gpio_cfg(
            LCD_SPI_SCL,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_pin_set(LCD_SPI_SCL);
	
	/*************SPI********************************************/
	// FlashоƬҪȳ՚MSBЈԫˤìҢȒ՚֚һٶʏʽҘӉҹc̹Ӕʨ׃Ђc 
	// CPOLۍCPHAּʨ׃ԉ1ҲˇࠉӔք,֢oּʨ׃ԉ0c
    NRF_SPI1->CONFIG =    (0<<0)    //MSB first
                        | (0<< 1)
                        | (0<< 2);
    NRF_SPI1->FREQUENCY = 0x80000000;     //8 Mbps
    NRF_SPI1->PSELSCK =  LCD_SPI_SCL;
    NRF_SPI1->PSELMOSI = LCD_SPI_SDA;
    //NRF_SPI1->PSELMISO = SPI2_MISO;
	
	NRF_SPI1->INTENCLR = ((1<<1)
	                    |(1<<4)
						|(1<<6)
						|(1<<8)
						|(1<<19)
						);
    
    NRF_SPI1->ENABLE = 0;
	
	/*****************************************************************/

	//4.DC
	// nrf_gpio_cfg_output(LCD_DC);
	
	
	nrf_gpio_cfg(
            LCD_DC,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_pin_clear(LCD_DC);	  
		
	//5.RES
	// nrf_gpio_cfg_output(LCD_RES);

	nrf_gpio_cfg(
            LCD_RES,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
	nrf_gpio_pin_set(LCD_RES);
	
	nrf_gpio_cfg_output(LCD_PWR);
	LCD_PWR_OFF();
}

#define spi1_transfer(data) do\
{\
	int timeout = 0;\
	NRF_SPI1->ENABLE = 1;\
	NRF_SPI1->EVENTS_READY = 0;\
    NRF_SPI1->TXD = data;\
    while ( ( NRF_SPI1->EVENTS_READY == 0) && (timeout++<5000) );\
    NRF_SPI1->EVENTS_READY = 0;\
	NRF_SPI1->ENABLE = 0;\
}while(0)

void WriteComm(unsigned char command)
{
	LCD_DC_CLR();
	
	LCD_SPI_CS_CLR();
	spi1_transfer( command );
	LCD_SPI_CS_SET();
}

void WriteData(unsigned char data)
{
	LCD_DC_SET();
	
	LCD_SPI_CS_CLR();
	spi1_transfer( data );
	LCD_SPI_CS_SET();
	
	LCD_DC_CLR();
}


void LCD_Init(void)
{
	LCD_SPI_Init();
	
	LCD_Config();
}

void LCD_Clear(void);

void LCD_Config(void)
{
	LCD_RES_SET();  
	nrf_delay_ms(2);
	
	LCD_RES_CLR();
	nrf_delay_ms(2);

	LCD_RES_SET();
	nrf_delay_ms(2);
	
	WriteComm(0x11);       //Sleep out

	nrf_delay_ms(5);       //Delay 120ms
	
#if (NEW_LCD_USED==1)
	WriteComm(0x21);
#endif	
    
	WriteComm(0xB1);     
	WriteData(0x01);   
	WriteData(0x08);   
	WriteData(0x05);   

	WriteComm(0xB2);     
	WriteData(0x01);   
	WriteData(0x08);   
	WriteData(0x05);   

	WriteComm(0xB3);     
	WriteData(0x01);   
	WriteData(0x08);   
	WriteData(0x05);   
	WriteData(0x05);   
	WriteData(0x08);   
	WriteData(0x05);   

	WriteComm(0xB4);     //Dot inversion
	WriteData(0x00);   

	WriteComm(0xC0);     
	WriteData(0x28);   
	WriteData(0x08);   
	WriteData(0x04);   

	WriteComm(0xC1);     
	WriteData(0xC0);

	WriteComm(0xC2);     
	WriteData(0x0D);   
	WriteData(0x00);   

	WriteComm(0xC3);     
	WriteData(0x8D);   
	WriteData(0x2A);   

	WriteComm(0xC4);     
	WriteData(0x8D);   
	WriteData(0xEE);   

	WriteComm(0xC5);     //VCOM
	WriteData(0x1D);   

	WriteComm(0x36);     //MX, MY, RGB mode
	WriteData(0xc8);   	 //c0

	WriteComm(0xE0);     
	WriteData(0x07);   
	WriteData(0x17);   
	WriteData(0x0C);   
	WriteData(0x15);   
	WriteData(0x2E);   
	WriteData(0x2A);   
	WriteData(0x23);   
	WriteData(0x28);   
	WriteData(0x28);   
	WriteData(0x28);   
	WriteData(0x2E);   
	WriteData(0x39);   
	WriteData(0x00);   
	WriteData(0x03);   
	WriteData(0x02);   
	WriteData(0x10);   

	WriteComm(0xE1);     
	WriteData(0x06);   
	WriteData(0x21);   
	WriteData(0x0D);   
	WriteData(0x17);   
	WriteData(0x35);   
	WriteData(0x30);   
	WriteData(0x2A);   
	WriteData(0x2D);   
	WriteData(0x2C);   
	WriteData(0x29);   
	WriteData(0x31);   
	WriteData(0x3B);   
	WriteData(0x00);   
	WriteData(0x02);   
	WriteData(0x03);   
	WriteData(0x12);   

	WriteComm(0x3A);     //65k mode
	WriteData(0x05);   
	nrf_delay_ms(5);

	LCD_Clear();	
}

void lcd_display( int enable)
{
	if ( enable == 0 )
	{
		WriteComm(0x28);
		WriteComm(0x10);
		LCD_PWR_OFF();		
		config.lcdEnable = 0;

		nrf_gpio_cfg_default(LCD_DC);
		nrf_gpio_cfg_default(LCD_SPI_SCL);
		nrf_gpio_cfg_default(LCD_SPI_SDA);
		nrf_gpio_cfg_default(LCD_RES);
		nrf_gpio_cfg_default(LCD_SPI_CS);
		
		config.enable = 0;
		lcd_enable_flag = 0;
		osal_start_timerEx( taskStepTaskId, TASK_STEP_BSTOTW_ENABLE_EVT, 500);
	} else
	{
		LCD_SPI_Init();
		LCD_PWR_ON();
		
		osal_set_event( taskGUITaskId, TASK_GUI_LCD_CONFIG_DELAY_EVT );
	}
}

void LCD_Config_Delay(void)
{
	LCD_Config();
	config.lcdEnable = 1;	
}

void LCD_Clear(void)
{
}

#include "Task_Motor.h"

void do_sync_frame( int from, int to )
{
	if ( config.lcdEnable == 0 )
	{
		return;
	}
	
#if (NEW_LCD_USED==1)
	WriteComm(0x2A);  //Set Column Address
	WriteData(0x00);
	WriteData(26);   //18
	WriteData(0x00);
	WriteData(105);  //67
	
	WriteComm(0x2B);  //Set Page Address
	WriteData(0x00);
	WriteData(from+1);
	WriteData(0x00);
	WriteData(to+1);    //	 
#else	
	WriteComm(0x2A);  //Set Column Address
	WriteData(0x00);
	WriteData(0x18);  //18
	WriteData(0x00);
	WriteData(0x67);  //67
	
	WriteComm(0x2B);  //Set Page Address
	WriteData(0x00);
	WriteData(from);
	WriteData(0x00);
	WriteData(to);    //	
#endif	
								  
	WriteComm(0x2c);	

	#if 0
	for ( int y = from; y < to; y++ )
	{
		for ( int x = 0; x < 80; x++ ) {
			WriteData(frame_buffer[y][x]>>8);
			WriteData(frame_buffer[y][x]&0xFF);
		}
	}	
	#else	
		
	register unsigned short *pval, val;
	for ( int y = from; y < to; y++ )
	{
		pval = &frame_buffer[y][0];		
		for ( int x = 0; x < 80; x++ ) 
		{	
            val = *pval++;
			LCD_SPI_CS_CLR();
			LCD_DC_SET();
			spi1_transfer(val>>8);
			LCD_DC_CLR();
			LCD_SPI_CS_SET();

			LCD_SPI_CS_CLR();			
			LCD_DC_SET();			
			spi1_transfer(val&0xFF);
			LCD_DC_CLR();
			LCD_SPI_CS_SET();
		}
	}			
	
	#endif
	
	if ( lcd_enable_flag == 0 && from == 140 && to == 160 )
	{
		WriteComm(0x29);     //Display on
		lcd_enable_flag = 1;
	}
}

#include "Task_Frame.h"

void sync_frame(void)
{
	osal_set_event( taskFrameTaskId, FRAME_TASK_SYNC_FRAME_EVT_0);
}
