#ifndef __BOARD_H__
#define __BOARD_H__

#define BMA_INT2_PIN        		11
#define BMA_SPI_CS_PIN    			 8
#define BMA_SPI_MOSI_PIN    		15   
#define BMA_SPI_MISO_PIN    		18   
#define BMA_SPI_SCL_PIN     		13

#define BMA_POWER_PIN               12
       
// #define BUTTON_PWR_PIN      		 2
#define BUTTON_PIN   				 2 

#define MOTOR_PWR_PIN        		25    // 27->25

#define BAT_DET_ADC_PIN  NRF_SAADC_INPUT_AIN4 /* 28 */
#define BAT_STAT_DET_PIN          	23
#define BAT_CHARGE_PIN 		     	24

#define FONT_SPI_CS_PIN    			20
#define FONT_SPI_MOSI_PIN    		15 
#define FONT_SPI_MISO_PIN    		18   
#define FONT_SPI_SCL_PIN     		13

#define HR_PWR_PIN                  31
#define HRS_SDA_PIN                 29
#define HRS_SCL_PIN                 27

// #define RX_PIN       			(11)
#define TX_PIN       	MOTOR_PWR_PIN  /* Output from this pin ^_^ */

#define LCD_DC        				3
#define LCD_SPI_CS    				30     // 29->30
#define LCD_SPI_SCL   				6
#define LCD_SPI_SDA   				7
#define LCD_RES       				5
#define LCD_PWR      				26     // 30->26

#define UITR_ADC_PIN                NRF_SAADC_INPUT_AIN2
#define UITR_ADC_CHANNEL            4



#endif
