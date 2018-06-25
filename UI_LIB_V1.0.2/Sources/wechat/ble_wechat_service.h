
#ifndef BLE_WECHAT_SERVICE
#define BLE_WECHAT_SERVICE

#include <string.h>
#include <stdint.h>
#include "ble.h"
#include "ble_conn_params.h"
#include "ble_types.h"

#define	COMPANY_IDENTIFIER							0x1234

#define BLE_UUID_WECHAT_SERVICE						0xFEE7
#define BLE_UUID_WECHAT_WRITE_CHARACTERISTICS 		0xFEC7
#define BLE_UUID_WECHAT_INDICATE_CHARACTERISTICS 	0xFEC8
#define BLE_UUID_WECHAT_READ_CHARACTERISTICS 		0xFEC9

#define BLE_UUID_WECHAT_PEDOMETER_CHARACTERISTICS   0xFEA1
#define BLE_UUID_WECHAT_TARGET_CHARACTERISTICS      0xFEA2

#define BLE_WECHAT_MAX_DATA_LEN (GATT_MTU_SIZE_DEFAULT - 3) 					/**<Set the Maximum Data length og WECHAT to the (GATT Maximum Transmission Unit -3) */

typedef struct
{
	uint16_t                   service_handle;
	uint16_t                   conn_handle;
	ble_gatts_char_handles_t   indicate_handles;
	ble_gatts_char_handles_t   write_handles;
	ble_gatts_char_handles_t   read_handles; 
	ble_gatts_char_handles_t   pedometer_indicate_handles;
	ble_gatts_char_handles_t   target_indicate_handles;
	bool is_indicate_enabled;
	bool pedometer_is_indicate_enabled;
	bool target_is_indicate_enabled;
	unsigned int indicate_counter;
	unsigned int indicate_comfirm_counter;
} ble_wechat_t;

extern ble_wechat_t 												m_ble_wechat;
extern uint8_t m_addl_adv_manuf_data [BLE_GAP_ADDR_LEN+2];

extern unsigned char m_pedometer_profile_val[4];

void ble_wechat_on_ble_evt(ble_wechat_t *p_wcs, ble_evt_t * p_ble_evt );

extern void wechat_services_init(ble_wechat_t *m_ble_wechat);
extern int wechat_indicate_pedometer( unsigned long pedometer );

#endif
