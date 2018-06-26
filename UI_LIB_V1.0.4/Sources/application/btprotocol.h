#ifndef __BT_PROTOCL_H__
#define __BT_PROTOCL_H__

extern int bt_protocol_rx(const unsigned char *buf, int length );
extern int bt_protocol_tx( unsigned char *buf, int length );

#endif
