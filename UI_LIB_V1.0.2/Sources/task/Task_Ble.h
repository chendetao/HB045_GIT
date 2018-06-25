#ifndef __TASK_BLE_H__
#define __TASK_BLE_H__

#define TASK_BLE_INIT_EVT                    0x00000001
#define TASK_BLE_SYSTEM_OFF_EVT              0x00000002
#define TASK_BLE_SYSTEM_REBOOT_EVT           0x00000004
#define TASK_BLE_WC_PEDOMETER_EVT            0x00000008
#define TASK_BLE_TIMEOUT_EVT                 0x00000010
#define TASK_BLE_CONNECTED_EVT               0x00000020
#define TASK_BLE_DISCONNECTED_EVT            0x00000040
#define TASK_BLE_SECURITY_EVT                0x00000080
#define TASK_BLE_TURN_OFF_ADV_EVT            0x00000100
#define TASK_BLE_PAIRED_FAILED_EVT           0x00000200
#define TASK_BLE_PAIRED_OK_EVT               0x00000400
#define TASK_BLE_WDT_FEED_EVT                0x00000800

#define ANCS_DISCOVERY_DELAY_START_EVT       0x00001000
#define TASK_BLE_MODIFY_DEVICE_NAME_EVT      0x00002000

#define TASK_BLE_STACK_INIT_EVT              0x00004000

extern unsigned long taskBleTaskId;

extern unsigned long taskBle( unsigned long task_id, unsigned long events );
extern void taskBleInit( unsigned long task_id );

#endif

