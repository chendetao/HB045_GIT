#ifndef __LCD_ST7735_H_
#define __LCD_ST7735_H_

#include "nrf_gpio.h"
#include "Board.h"


 // LCD width and height
#define LCD_W     80
#define LCD_H    160

#define LCD_DC_SET()           nrf_gpio_pin_set(LCD_DC)
#define LCD_DC_CLR()           nrf_gpio_pin_clear(LCD_DC)

#define LCD_SPI_CS_SET()       nrf_gpio_pin_set(LCD_SPI_CS)
#define LCD_SPI_CS_CLR()       nrf_gpio_pin_clear(LCD_SPI_CS)

#define LCD_RES_SET()          nrf_gpio_pin_set(LCD_RES)
#define LCD_RES_CLR()          nrf_gpio_pin_clear(LCD_RES)

#define LCD_PWR_OFF()          nrf_gpio_pin_set(LCD_PWR)
#define LCD_PWR_ON()          nrf_gpio_pin_clear(LCD_PWR)        

extern void LCD_Init(void);
extern void LCD_Config_Delay(void);
extern void Address_set(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
extern void BlockWrite(int pic);
extern void lcd_display(int enable);
extern void sync_frame(void);

extern void do_sync_frame( int from, int to );

#endif


