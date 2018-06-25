/**************************************************************************
* Copytight 2017 Mandridy Tech.Co., Ltd. All rights reserved              *
*                                                                         *
* Filename : BMA_SPI.h                                                    *
* Author : Wen Shifang                                                    *
* Version : 1.0                                                           *
*                                                                         *
* Decription : SPI Interface function implementation for BMA2X.           *
*               header file for BMA_SPI.c                                 *
* Function-list:                                                          *
* History:                                                                *
*       <author>     <time>     <version>  <desc>                         *
*       Wen Shifang  2017-4.6   1.0        build this moudle              *
*                                                                         *
*                                                                         *
* Created : 2017-4-6                                                      *
* Last modified : 2017.4.6                                                *
**************************************************************************/

#ifndef __SPI_HW_H__
#define __SPI_HW_H__

#include "nrf_gpio.h"

extern void spi_configure( NRF_SPI_Type *spi, int scl, int mosi, int miso);
extern unsigned int spi_transfer(NRF_SPI_Type *spi, unsigned int val);
extern void spi_write_bytes( NRF_SPI_Type *spi, unsigned char *buf, int length );
extern inline void spi_write_byte( NRF_SPI_Type *spi, unsigned char val );

#endif

