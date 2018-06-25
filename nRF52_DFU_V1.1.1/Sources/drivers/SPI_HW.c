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
    /**
	 * CPOL = 0 CPHA = 0
	 */
    spi->CONFIG    = (0<<0)    	// MSB first
                        | (1<< 1)
                        | (1 << 2);
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
 * SPI transfer send a byte then receive a byte
 */
unsigned int spi_transfer(NRF_SPI_Type *spi, unsigned int val)
{
	int timeout = 6;
	
	spi->ENABLE = 1;	
	
    spi->TXD = val & 0xFF;
	// Waiting for ready
    while ( (spi->EVENTS_READY == 0) && (timeout-->0) );
    spi->EVENTS_READY = 0;

    val = spi->RXD;
	
	spi->ENABLE = 0; 
	
	return val;
}

int write_clt = 0;
/**
 * SPI transfer send bytes
 */
void spi_write_bytes( NRF_SPI_Type *spi, unsigned char *p, int length )
{
	int timeout;
	
	spi->ENABLE = 1;
	
	write_clt = 0;
	
	for ( int i = length; i != 0; i-- )
	{
		timeout = 10;
		spi->TXD = *p--;
		while( (spi->EVENTS_READY == 0) && (timeout-->0) );
		spi->EVENTS_READY = 0;
		
		write_clt++;
	}
	
	spi->ENABLE = 0;
}

/**
 * function protype : void spi_write_byte(unsigned char addr,
 *                                            unsigned char data)
 * description : write a byte data into the module register by spi bus
 * addr : register address on module
 * data : the data to write
 * return value : none
 */
inline void spi_write_byte( NRF_SPI_Type *spi, unsigned char val )
{
	int timeout = 100;
	
	spi->ENABLE = 1;
	
	spi->TXD = val;
	while(( spi->EVENTS_READY == 0 ) && ( timeout-->0 ));
	spi->EVENTS_READY = 0;
	
	spi->ENABLE = 0;
}

/**
 * function protype : unsigned char spi_read_byte( unsigned char addr )
 * description : read a byte data from the module register by spi bus
 * addr : register address on module
 * return value : data read from module
 */
inline unsigned char spi_read_byte( void )
{
    unsigned char val = 0;

    return val;
}
