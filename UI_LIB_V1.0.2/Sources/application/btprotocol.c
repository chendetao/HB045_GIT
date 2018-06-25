
#include "string.h"

#include "OSAL.h"
#include "OSAL_Timer.h"
#include "Task_Step.h"
#include "Task_Upload.h"
#include "Task_GUI.h"
#include "Task_Ble.h"
#include "Task_Flash.h"

#include "btprotocol.h"
#include "ClockTick.h"
#include "Alarm.h"
#include "UserSetting.h"
#include "version.h"
#include "MsgQueue.h"
#include "Ancs.h"
#include "Log.h"
#include "Window.h"
#include "UI_win_notify.h"
#include "Task_GUI.h"
#include "Task_Motor.h"
#include "Task_Flash.h"
#include "Task_Hr.h"

extern int ble_write_bytes( unsigned char *buf, int length );

int bt_protocol_tx( unsigned char *buf, int length )
{
	return ble_write_bytes(buf, length);
}

int bt_protocol_rx(const unsigned char *buf, int length )
{
	unsigned char txbuf[20];
	
	memset( txbuf, 0, sizeof(txbuf) );
	
	txbuf[0] = buf[1];

	/**
	 * Make a response to client
	 */
	bt_protocol_tx( txbuf, sizeof(txbuf));
	
	return 0;
}

void notify_remove(void);


void notify_remove(void)
{
	 win_ticks = 1000;
}
