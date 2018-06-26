#ifndef _F_LIB_H___
#define _F_LIB_H___

#include "SPI_HW.h"

#define CS_FONT_ENABLE() nrf_gpio_pin_clear(FONT_SPI_CS_PIN)
#define CS_FONT_DISABLE()  nrf_gpio_pin_set(FONT_SPI_CS_PIN)

extern void font_init(void);

extern void EntryDeepSleepMode(void);
extern void ExitDeepSleepMode(void);

#endif
