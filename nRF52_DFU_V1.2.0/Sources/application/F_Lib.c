
#include "nrf_gpio.h"
#include "Board.h"
#include "SPI_HW.h"
#include "nrf52.h"
#include "F_Lib.h"

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

void WriteEnable(void)
{
    CS_FONT_ENABLE();
    
    spi_write_byte(NRF_SPI0, 0x06);

    CS_FONT_DISABLE();    
}

void WriteDisable(void)
{
    CS_FONT_ENABLE();
    
    spi_write_byte(NRF_SPI0, 0x04);

    CS_FONT_DISABLE();  
}

void EraseSector( unsigned long addr )    
{
    ExitDeepSleepMode();
    
    WriteEnable();
 
    CS_FONT_ENABLE();
   
    //发送命令及地址
    spi_write_byte( NRF_SPI0, 0x20 );                /* Command   */
    spi_write_byte( NRF_SPI0,(addr>>16) & 0xFF);    /* Address 1 */
    spi_write_byte( NRF_SPI0,(addr>> 8) & 0xFF);    /* Address 2 */
    spi_write_byte( NRF_SPI0,(addr>> 0) & 0xFF);    /* Address 3 */    
    
    CS_FONT_DISABLE();
    
    WriteDisable();
    
    EntryDeepSleepMode();
}

void WritePage( unsigned long addr, unsigned char *buf, int len )
{
    ExitDeepSleepMode();
    
    WriteEnable();
 
    CS_FONT_ENABLE();
   
    //发送命令及地址
    spi_write_byte( NRF_SPI0, 0x02 );                /* Command   */
    spi_write_byte( NRF_SPI0,(addr>>16) & 0xFF);    /* Address 1 */
    spi_write_byte( NRF_SPI0,(addr>> 8) & 0xFF);    /* Address 2 */
    spi_write_byte( NRF_SPI0,(addr>> 0) & 0xFF);    /* Address 3 */    
    
    for ( int i = 0; i < len; i++ )
    {
        spi_write_byte(NRF_SPI0, buf[i]);
    }
    
    CS_FONT_DISABLE();
    
    WriteDisable();
    
    EntryDeepSleepMode();    
}

void ReadPage( unsigned long addr, unsigned char *buf, int len )
{
    ExitDeepSleepMode();
    
    CS_FONT_ENABLE();
    
    //发送命令及地址
    spi_write_byte( NRF_SPI0, 0x0B );               /* Command   */
    spi_write_byte( NRF_SPI0,(addr>>16) & 0xFF);    /* Address 1 */
    spi_write_byte( NRF_SPI0,(addr>> 8) & 0xFF);    /* Address 2 */
    spi_write_byte( NRF_SPI0,(addr>> 0) & 0xFF);    /* Address 3 */    
    spi_write_byte( NRF_SPI0,0xA5);
    
    for ( int i = 0; i < len; i++ )
    {
        buf[i] = spi_transfer(NRF_SPI0, 0xA5);
    }
    
    CS_FONT_DISABLE();
    
    EntryDeepSleepMode();    
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

/**
 * void font_init(void);
 * Set the SPI pin SCLK, MISO, MOSI And
 * Initialize the SPI Master. 
 */
void font_init(void)
{   
	/**
	 * SPI MOSI
	 */
	nrf_gpio_cfg(
            FONT_SPI_MOSI_PIN,
            NRF_GPIO_PIN_DIR_OUTPUT,
            NRF_GPIO_PIN_INPUT_DISCONNECT,
            NRF_GPIO_PIN_PULLUP,
            NRF_GPIO_PIN_S0S1,
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
            NRF_GPIO_PIN_S0S1,
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
            NRF_GPIO_PIN_S0S1,
            NRF_GPIO_PIN_NOSENSE);	
            
	nrf_gpio_cfg_output(FONT_SPI_CS_PIN);
	CS_FONT_DISABLE();

    spi_configure ( NRF_SPI0, FONT_SPI_SCL_PIN, FONT_SPI_MOSI_PIN, FONT_SPI_MISO_PIN );
}
