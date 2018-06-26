/**************************************************************************
* Copytight 2017 Mandridy Tech.Co., Ltd. All rights reserved              *
*                                                                         *
* Filename : BMA_SPI.c                                                    *
* Author : Wen Shifang                                                    *
* Version : 1.0                                                           *
*                                                                         *
* Decription : SPI Interface function implementation for BMA2X.           *
* Function-list:                                                          *
* History:                                                                *
*       <author>     <time>     <version>  <desc>                         *
*       Wen Shifang  2017-4.6   1.0        build this moudle              *
*                                                                         *
*                                                                         *
* Created : 2017-4-6                                                      *
* Last modified : 2017.4.6                                                *
**************************************************************************/

#include "nrf_gpio.h"
#include "nrf_spi.h"
#include "SPI_HW.h"
#include "Board.h"

void spi_configure( NRF_SPI_Type *spi, int scl, int mosi, int miso)
{
}

/**
 * function protype : void spi_write_byte(unsigned char addr,
 *                                            unsigned char data)
 * description : write a byte data into the module register by spi bus
 * addr : register address on module
 * data : the data to write
 * return value : none
 */
void spi_write_byte( NRF_SPI_Type *spi, unsigned char val )
{    
    for ( int i = 0; i < 8; i++ )
    {
        if ( val & 0x80 )
        {
            nrf_gpio_pin_set(FONT_SPI_MOSI_PIN);
        }else{
            nrf_gpio_pin_clear(FONT_SPI_MOSI_PIN);
        }
        __nop();__nop();
        __nop();__nop();        
        
        nrf_gpio_pin_set(FONT_SPI_SCL_PIN);    
        val <<= 1;
        __nop();__nop();
        __nop();__nop();
        nrf_gpio_pin_clear(FONT_SPI_SCL_PIN);
        __nop();__nop();
        __nop();__nop();
    }
    
    nrf_gpio_pin_set(FONT_SPI_MOSI_PIN);
}

/**
 * function protype : unsigned char spi_read_byte( unsigned char addr )
 * description : read a byte data from the module register by spi bus
 * addr : register address on module
 * return value : data read from module
 */
unsigned char spi_read_byte( void )
{
    unsigned char val = 0;

    for ( int i = 0; i < 8; i++ )
    {
        nrf_gpio_pin_set(FONT_SPI_SCL_PIN);        
     
        val <<= 1;
        __nop();__nop();   
        __nop();__nop();        

        if ( nrf_gpio_pin_read(FONT_SPI_MISO_PIN) )
        {
            val |= 0x1;
        }  

        nrf_gpio_pin_clear(FONT_SPI_SCL_PIN);           
        __nop();__nop();
        __nop();__nop();        
    }    
    
    return val;
}
