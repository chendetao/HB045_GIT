
#include "nrf_gpio.h"
#include "Board.h"
#include "SPI_HW.h"
#include "nrf52.h"
#include "SPI.h"

#define CS_FONT_ENABLE() nrf_gpio_pin_clear(FONT_SPI_CS_PIN)
#define CS_FONT_DISABLE()  nrf_gpio_pin_set(FONT_SPI_CS_PIN)

void EntryDeepSleepMode(void)
{
    CS_FONT_ENABLE(); 
    
    spi_write_byte(NRF_SPI0, 0xB9);
        
    CS_FONT_DISABLE();
}

void ExitDeepSleepMode(void)
{
    CS_FONT_ENABLE();
    
    spi_write_byte(NRF_SPI0, 0xAB);

    CS_FONT_DISABLE();
}

void r_dat_bat(unsigned long TAB_addr,unsigned int Num,unsigned char *p_arr)
{
    unsigned long j = 0;
	
    CS_FONT_ENABLE();
   
    //发送命令及地址
    spi_write_byte( NRF_SPI0, 0x3 );               /* Command   */
    spi_write_byte( NRF_SPI0,(TAB_addr>>16) & 0xFF);    /* Address 1 */
    spi_write_byte( NRF_SPI0,(TAB_addr>> 8) & 0xFF);    /* Address 2 */
    spi_write_byte( NRF_SPI0,(TAB_addr>> 0) & 0xFF);    /* Address 3 */

    for( j = 0; j < Num; j++ )
    {
        p_arr[j] = spi_transfer(NRF_SPI0, 0xFF);    //取点阵数据
    }
    
    CS_FONT_DISABLE();
}

static void spi_configure_( NRF_SPI_Type *spi, int scl, int mosi, int miso)
{
    /**
	 * CPOL = 0 CPHA = 0
	 */
    spi->CONFIG    = (0<<0)    	// MSB first
                        | (0<< 1)
                        | (0 << 2);
    spi->FREQUENCY = 0x80000000; 	// 8 Mbps
    spi->PSELSCK   = scl;
    spi->PSELMOSI  = mosi;
    spi->PSELMISO  = miso;
	
	spi->INTENCLR  = ((1<<1)
	                    |(1<<4)
						|(1<<6)
						|(1<<8)
						|(1<<19)
						);
    
    spi->ENABLE    = 0;			// Not Enable here
}

/**
 * void init_spi_master(void);
 * Set the SPI pin SCLK, MISO, MOSI And
 * Initialize the SPI Master. 
 */
void font_init_spi_pin(void)
{   
	/**
	 * SPI MOSI
	 */
	nrf_gpio_cfg(
            FONT_SPI_MOSI_PIN,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_H0H1,
            NRF_GPIO_PIN_NOSENSE);
    
	nrf_gpio_pin_set(FONT_SPI_MOSI_PIN);

	/**
	 * SPI SCL
	 */
	nrf_gpio_cfg(
            FONT_SPI_SCL_PIN,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_H0H1,
            NRF_GPIO_PIN_NOSENSE);
    
	nrf_gpio_pin_set(FONT_SPI_SCL_PIN);
	
	/**
	 * SPI MISO
	 */	
    nrf_gpio_cfg(
            FONT_SPI_MISO_PIN,
            NRF_GPIO_PIN_DIR_INPUT,
            NRF_GPIO_PIN_INPUT_CONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_H0H1,
            NRF_GPIO_PIN_NOSENSE);	
			
	nrf_gpio_cfg_output(FONT_SPI_CS_PIN);
	CS_FONT_DISABLE();			
}

void font_init(void)
{
	font_init_spi_pin();
	
	spi_configure_(NRF_SPI0, FONT_SPI_SCL_PIN, FONT_SPI_MOSI_PIN, FONT_SPI_MISO_PIN);
}
