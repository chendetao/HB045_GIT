
#include "Debug.h"

#include "nrf_gpio.h"
#include "Board.h"
#include "btprotocol.h"

void Debug_uart_init(void){
   
    //nrf_gpio_cfg_input(RX_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_output(TX_PIN);
   
    //NRF_UART0->PSELRXD = RX_PIN;      
    NRF_UART0->PSELTXD = TX_PIN;
    NRF_UART0->PSELRTS = 0XFFFFFFFF;
    NRF_UART0->PSELCTS = 0XFFFFFFFF;
   
    NRF_UART0->BAUDRATE = 0x01D7E000; // 115200
    NRF_UART0->CONFIG = 0;       // no hardware flow control no parity. 
   
    NRF_UART0->EVENTS_TXDRDY = 0;  
   
    NRF_UART0->ENABLE = 4;          // enable uart
    NRF_UART0->TASKS_STARTTX = 1;   //
}

void Debug_uart_send_byte(unsigned char data)
{
    unsigned char temp = data;

    NRF_UART0->EVENTS_TXDRDY = 0;
    NRF_UART0->TXD = temp;

    while(NRF_UART0->EVENTS_TXDRDY == 0){
       ;
    }
}

void Debug_uart_send_hex(unsigned long val)
{
	unsigned char buf[4];
	unsigned long sum;
	
	buf[0] = 0xCC;
	buf[1] = (val >> 8) & 0xFF;
	buf[2] = val & 0xFF;

	sum = buf[0] + buf[1] + buf[2];
	
	buf[3] = sum & 0xFF;
	
	for ( int i = 0; i < 4; i++ )
	{
		NRF_UART0->EVENTS_TXDRDY = 0;
		NRF_UART0->TXD = buf[i];
		
		while(NRF_UART0->EVENTS_TXDRDY == 0){
		   ;
		}		
	}
}

unsigned char ble_tx_buf[20];
int length = 0;
unsigned int sendIdx = 0;
extern short current_touch_adc;

void Debug_ble_send_hex(unsigned long val)
{
	ble_tx_buf[0] = 0x0F;
	ble_tx_buf[1] = 0x0F;
	ble_tx_buf[2] = sendIdx&0xFF;
		
	if ( length == 0 )
	{
		ble_tx_buf[3] = (val >> 8) & 0xFF;
		ble_tx_buf[4] = val & 0xFF;
		
		length = 1;
	} else if ( length == 1 )
	{
		ble_tx_buf[5] = (val >> 8) & 0xFF;
		ble_tx_buf[6] = val & 0xFF;	
		length = 2;
	} else if ( length == 2 )
	{
		ble_tx_buf[7] = (val >> 8) & 0xFF;
		ble_tx_buf[8] = val & 0xFF;	
		length = 3;		
	}else if ( length == 3 )
	{
		ble_tx_buf[9] = (val >> 8) & 0xFF;
		ble_tx_buf[10] = val & 0xFF;

		length = 4;
	}else if ( length == 4 )
	{
		ble_tx_buf[11] = (val >> 8) & 0xFF;
		ble_tx_buf[12] = val & 0xFF;	
		length = 5;		
	} else if ( length == 5 )
	{
		ble_tx_buf[13] = (val >> 8) & 0xFF;
		ble_tx_buf[14] = val & 0xFF;	
		length = 6;		
	}else if ( length == 6 )
	{
		ble_tx_buf[15] = (val >> 8) & 0xFF;
		ble_tx_buf[16] = val & 0xFF;	

		ble_tx_buf[17] = (current_touch_adc >> 8) & 0xFF;
		ble_tx_buf[18] = current_touch_adc & 0xFF;	
		
		unsigned long sum = 0;
		for ( int i = 0; i < 16; i++ )
		{
           sum += ble_tx_buf[i+3];
		}	
		ble_tx_buf[19] = sum & 0xFF;
		length = 0;	

		sendIdx++;
		bt_protocol_tx(ble_tx_buf, sizeof(ble_tx_buf));		
	}
}

