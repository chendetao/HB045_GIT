#ifndef __DEBUG_H__
#define __DEBUG_H__

#define ENABLE_DEBUG_OUTPUT  0

void Debug_uart_send_byte(unsigned char data);
void Debug_uart_send_hex(unsigned long val);
void Debug_ble_send_hex(unsigned long val);

void Debug_uart_init(void);


#endif
