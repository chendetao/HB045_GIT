/**************************************************************************
* Copytight 2017 Mandridy Tech.Co., Ltd. All rights reserved              *
*                                                                         *
* Filename : BMA253.c                                                     *
* Author : Wen Shifang                                                    *
* Version : 1.0                                                           *
*                                                                         *
* Decription : BMA GSensor Init                                           *
* Function-list:                                                          *
* History:                                                                *
*       <author>     <time>     <version>  <desc>                         *
*       Wen Shifang  2017-4.6   1.0        build this moudle              *
*                                                                         *
*                                                                         *
* Created : 2017-4-6                                                      *
* Last modified : 2017.4.6                                                *
**************************************************************************/

/*******************************************************************************
* Copytight 2016 Mandridy Tech.Co., Ltd. All rights reserved 
*
* Filename : bma250.c
* Author : Wen Shifang
* Version : 1.0
*
* Decription : File battery_task.c implements the battery level detect function.
* Function-list:
* History:
*       <author>     <time>    <version>  <desc>
*       Wen Shifang  2016-1.22 1.0        build this moudle
*
* Created : 2015-11-1
* Last modified : 2016.1.22
*******************************************************************************/
#include "bma2x2.h"
#include "bma253.h"
#include "SPI_HW.h"
#include "nrf_gpio.h"
#include "nrf52.h"
#include "OSAL.h"

#include "Task_Step.h"

#include "Board.h"
#include "nrf_delay.h"

struct bma2x2_t bma2x2;

static void BMA2x2_delay_msek(u32 msek);

static signed char BMA2x2_SPI_bus_write(unsigned char dev_addr,\
	unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt);
static signed char BMA2x2_SPI_bus_read(unsigned char dev_addr,\
	unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt);

volatile int error_cnt = 0;

static void bma_int_init(void);

#define CS_BMA_ENABLE() nrf_gpio_pin_clear(BMA_SPI_CS_PIN)
#define CS_BMA_DISABLE()  nrf_gpio_pin_set(BMA_SPI_CS_PIN)

#define BMA_POWER_DOWN()  nrf_gpio_pin_clear(BMA_POWER_PIN)
#define BMA_POWER_ON()    nrf_gpio_pin_set(BMA_POWER_PIN)

/**
 * void init_spi_master(void);
 * Set the SPI pin SCLK, MISO, MOSI And
 * Initialize the SPI Master. 
 */
void BMA_init_spi_pin(void)
{   
	/**
	 * SPI MOSI
	 */
	nrf_gpio_cfg(
            BMA_SPI_MOSI_PIN,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
    
	nrf_gpio_pin_clear(BMA_SPI_MOSI_PIN);

	/**
	 * SPI SCL
	 */
	nrf_gpio_cfg(
            BMA_SPI_SCL_PIN,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
    
	nrf_gpio_pin_clear(BMA_SPI_SCL_PIN);
	
	/**
	 * SPI MISO
	 */
	nrf_gpio_cfg(
            BMA_SPI_MISO_PIN,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
    
	nrf_gpio_pin_clear(BMA_SPI_MISO_PIN);
			
	nrf_gpio_cfg_output(BMA_SPI_CS_PIN);
	nrf_gpio_pin_clear(BMA_SPI_CS_PIN);	

	////////////////////////////////////////////////////////
	//                                                    //
	// 前面的先把IO口全部拉低，然后电源关闭，delay 2ms后  //
	// 再恢复供电，IO口恢复状态                           //
	////////////////////////////////////////////////////////
	
	nrf_gpio_cfg_output(BMA_POWER_PIN);
	BMA_POWER_DOWN();
	
	nrf_delay_ms(20);
	
	BMA_POWER_ON();
	nrf_delay_ms(5);	
	
	nrf_gpio_pin_set(BMA_SPI_MOSI_PIN);
	nrf_gpio_pin_set(BMA_SPI_SCL_PIN);
	nrf_gpio_pin_set(BMA_SPI_CS_PIN);

	/**
	 * SPI MISO
	 */	
    nrf_gpio_cfg(
            BMA_SPI_MISO_PIN,
            NRF_GPIO_PIN_DIR_INPUT,
            NRF_GPIO_PIN_INPUT_CONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);
}

void make_bma250_to_deep_sleep( int enable)
{   
    unsigned int cnt = 0;
    
    if ( enable ==  1 )
    {      
        unsigned char powerMode = 0;
        do { 
            bma2x2_set_power_mode(BMA2x2_MODE_DEEP_SUSPEND);
            
            bma2x2_get_power_mode( &powerMode );
        }while( powerMode != BMA2x2_MODE_DEEP_SUSPEND && (++cnt<32));
    }else
    {
        bma250_init();
    }
}

int bma250_init( void )
{
    unsigned char ret, cnt = 0;
    unsigned char BackRead = 0;
	
	error_cnt = 0;
		
	BMA_init_spi_pin();		
	bma_int_init();
	spi_configure(NRF_SPI0, BMA_SPI_SCL_PIN, BMA_SPI_MOSI_PIN, BMA_SPI_MISO_PIN);

	bma2x2.bus_write  = BMA2x2_SPI_bus_write;
	bma2x2.bus_read   = BMA2x2_SPI_bus_read;
    bma2x2.delay_msec = BMA2x2_delay_msek;
	
    bma2x2_init(&bma2x2);
    
    bma2x2_soft_rst();
    BMA2x2_delay_msek(10);
    
    bma2x2_soft_rst();
    BMA2x2_delay_msek(10);
    
    cnt = 0;
    /* Set PowerMode to BMA2x2_MODE_LOWPOWER1 */
    do { 
        bma2x2_set_power_mode(BMA2x2_MODE_NORMAL);
        bma2x2_get_power_mode( &BackRead );
    }while( BackRead != BMA2x2_MODE_NORMAL && (++cnt<32));
    if( cnt >= 32 )
      error_cnt++;
    
    BMA2x2_delay_msek(10);
    
	/* software reset (ALL Register realod default values )*/
    bma2x2_soft_rst(); 
    BMA2x2_delay_msek(10);

	/* software reset (ALL Register realod default values )*/
    bma2x2_soft_rst(); 
    BMA2x2_delay_msek(10);
    
    
     /* Configure the BMA250E work at FIFO mode */
    cnt = 0;
    do{
        bma2x2_set_bw(BMA2x2_BW_500HZ);
        bma2x2_get_bw( &BackRead );
    }while( BackRead != BMA2x2_BW_500HZ && (++cnt<32));
    if( cnt >= 32 )
      error_cnt++;

    /*�ж��ź�Ϊ�������ģʽ */
    cnt = 0;
    do{
        bma2x2_set_intr_output_type(1, 0);          
        bma2x2_get_intr_output_type(1, &BackRead);
    }while( BackRead != 0  && (++cnt<32));
    if( cnt >= 32 )
      error_cnt++;

     /* �ж��ź�Ϊ�ߵ�ƽ */
    cnt = 0;
    do{
        bma2x2_set_intr_level(1, ACTIVE_HIGH);  
        bma2x2_get_intr_level(1, &BackRead);
    }while( BackRead != ACTIVE_HIGH && (++cnt<32));
    if( cnt >= 32 )
      error_cnt++;

    /* Clear any latch interrupt */
    cnt = 0;
    do{
        ret = bma2x2_rst_intr(0x1);  
    }while( ret != 0 && (++cnt<32)); 
    if( cnt >= 32 )
      error_cnt++;
        
	/** int signal latch 1ms */
    cnt = 0;
    do{
        bma2x2_set_latch_intr(BMA2x2_LATCH_DURN_1MS);
        bma2x2_get_latch_intr( &BackRead );
    }while( BackRead != BMA2x2_LATCH_DURN_1MS && (++cnt<32)); 
    if( cnt >= 32 )
      error_cnt++;

    cnt = 0;
    do{
        bma2x2_set_range(BMA2x2_RANGE_2G);
        bma2x2_get_range( &BackRead );
    }while( BackRead != BMA2x2_RANGE_2G && (++cnt<32));
    if( cnt >= 32 )
      error_cnt++;
        
    /* FIFO accel data selected : XYZ in FIFO */
    cnt = 0;
    do{
        bma2x2_set_fifo_data_select(0x0);
        bma2x2_get_fifo_data_select( &BackRead );
    }while( BackRead != 0x0 && (++cnt<32)); 
    if( cnt >= 32 )
      error_cnt++;

     /* FIFO mode : stream mode  */
    cnt = 0;
    do{
        bma2x2_set_fifo_mode(2); 
        bma2x2_get_fifo_mode( &BackRead );
    }while( BackRead != 2 && (++cnt<32));
    if( cnt >= 32 )
      error_cnt++;

    //// water marker level 20 ///////////////////
    cnt = 0;
    do{
        bma2x2_set_fifo_wml_trig(FIFO_DEPTH);
        bma2x2_get_fifo_wml_trig( &BackRead );
    }while( BackRead != FIFO_DEPTH && (++cnt<32)); 
    if( cnt >= 32 )
      error_cnt++;

    cnt = 0;
    do{
        bma2x2_set_intr_fifo_wm(INTR_ENABLE);
        bma2x2_get_intr_fifo_wm( &BackRead );
    }while( BackRead != INTR_ENABLE && (++cnt<32));
    if( cnt >= 32 )
      error_cnt++;

    cnt = 0;
    do{
        bma2x2_set_intr2_fifo_wm(INTR_ENABLE);       
        bma2x2_get_intr2_fifo_wm( &BackRead );
    }while( BackRead != INTR_ENABLE && (++cnt<32)); 
    if( cnt >= 32 )
      error_cnt++;


    cnt = 0;
    do{
        bma2x2_set_sleep_durn(BMA2x2_SLEEP_DURN_25MS);   
        bma2x2_get_sleep_durn( &BackRead );
    }while( BackRead != BMA2x2_SLEEP_DURN_25MS && (++cnt<32));
    if( cnt >= 32 )
      error_cnt++;

    cnt = 0;
    do{
        bma2x2_set_sleep_timer_mode(1);  /* EST mode */
        bma2x2_get_sleep_timer_mode( &BackRead );
    }while( BackRead != 1 && (++cnt<32)); 
    if( cnt >= 32 )
      error_cnt++;

    /* Set PowerMode to BMA2x2_MODE_LOWPOWER1 */
    cnt = 0;
    do { 
        bma2x2_set_power_mode(BMA2x2_MODE_LOWPOWER1); 
        bma2x2_get_power_mode( &BackRead );
    }while( BackRead != BMA2x2_MODE_LOWPOWER1 && (++cnt<32));
    if( cnt >= 32 )
      error_cnt++;
    
    return ( error_cnt != 0 )?(-1):(0);
}


static signed char BMA2x2_SPI_bus_write(unsigned char dev_addr,
	unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt)
{
	int iError = 0;

	unsigned int i = 0;

	CS_BMA_ENABLE();
	
	NRF_SPI0->ENABLE = 1; 
		
	//When 0, the data SDI is written into the chip
	spi_transfer( NRF_SPI0, (reg_addr)|0x00 ); 
	for( i = 0; i < cnt; i++ )
	{
		spi_transfer( NRF_SPI0, reg_data[i]);
	}
		
	NRF_SPI0->ENABLE = 0;
	
	CS_BMA_DISABLE();
	
	return (s8)iError;
}

static signed char BMA2x2_SPI_bus_read(unsigned char dev_addr, 
	unsigned char reg_addr, unsigned char *reg_data, unsigned char cnt)
{
	int iError = 0;

	unsigned int i = 0;
	
	CS_BMA_ENABLE();
	
	NRF_SPI0->ENABLE = 1;
	
	//When 1, the data SDO from the chip is read.
	spi_transfer( NRF_SPI0, (reg_addr)|0x80); 
	for ( i = 0; i < cnt; i++ )
	{
		reg_data[i] = spi_transfer( NRF_SPI0, 0xff) & 0xFF;
	}

	NRF_SPI0->ENABLE = 0;	
	
	CS_BMA_DISABLE();
	
	return (s8)iError;
}

/*	Brief : The delay routine
 *	\param : delay in ms
*/
void BMA2x2_delay_msek(u32 msek)
{
    /*Here you can write your own delay routine*/
	
    int t = 1000*msek;
	
    while (t-->0)
    {
		__asm("nop");
    }
}

void bma_int_init(void)
{ 	
	nrf_gpio_pin_pull_t config = NRF_GPIO_PIN_PULLDOWN;
    nrf_gpio_cfg_input(BMA_INT2_PIN, config);
	
	// GPIOTE0 chanenl 0 as event
	// for GPIO pin BMA_INT2_PIN (23)
	// Rasing trigger interrupt
    NRF_GPIOTE->CONFIG[0] = 1 << 0
                     |(BMA_INT2_PIN << 8)
                     |(1 << 16);
	// Enable interrupt
	NRF_GPIOTE->INTENSET = 0x1;
	  
	// Set interrupt priority
    NVIC_SetPriority(GPIOTE_IRQn, 1);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_EnableIRQ(GPIOTE_IRQn);	
}

// Note GPIOTE_IRQHandler Defined at IRQHandler.c : GPIOTE_Handler
