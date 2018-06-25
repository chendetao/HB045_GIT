#ifndef __SSADC_H___
#define __SSADC_H___

#include "Board.h"
#include "nrf_saadc.h"

#define SAADC_BUFF_SISE  8

extern void saadc_init(void);
nrf_saadc_value_t* saadc_buffer_convert(void);

#endif
