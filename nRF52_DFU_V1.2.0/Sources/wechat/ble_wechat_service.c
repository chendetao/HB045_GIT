/*
*	modified  by anqiren  2014/12/02  V1.0bat
*
**/

#include "ble_wechat_service.h"
#include "nrf_gpio.h"
#include "ble_conn_params.h"
#include "ble_types.h"
#include "app_error.h"

uint8_t m_addl_adv_manuf_data [BLE_GAP_ADDR_LEN+2];
ble_wechat_t 													m_ble_wechat;

static uint32_t ble_wechat_add_service(ble_wechat_t *p_wechat);
static uint32_t ble_wechat_add_characteristics(ble_wechat_t *p_wechat);

/*****************************************************************************
* data handle
*****************************************************************************/
 
unsigned char m_write_char_val[1];
/** unsigned char m_read_char_val[6]; */ /* Using m_addl_adv_manuf_data */
unsigned char m_read_indicate_val[1];
unsigned char m_pedometer_profile_val[4];
unsigned char m_target_profile_val[4];
 
/**@brief     Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_wechat_t *p_wcs, ble_evt_t * p_ble_evt )
{
	ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	
    if (
        (p_evt_write->handle == p_wcs->indicate_handles.cccd_handle)
        &&
        (p_evt_write->len == 2)
       )
    {
        if (ble_srv_is_indication_enabled(p_evt_write->data))
        {
            p_wcs->is_indicate_enabled = true;
        }
        else
        {
            p_wcs->is_indicate_enabled = false;
        }
    }else if(
		(p_evt_write->handle == p_wcs->pedometer_indicate_handles.cccd_handle )
		&&
		(p_evt_write->len == 2) )
	{
		if (  ble_srv_is_indication_enabled(p_evt_write->data))
		{
			p_wcs->pedometer_is_indicate_enabled = true;
		}else
		{
			p_wcs->pedometer_is_indicate_enabled = false;
		}
	}else if(
		(p_evt_write->handle == p_wcs->target_indicate_handles.cccd_handle )
		&&
		(p_evt_write->len == 2) )
	{
		if (  ble_srv_is_indication_enabled(p_evt_write->data))
		{
			p_wcs->target_is_indicate_enabled = true;
		}else
		{
			p_wcs->target_is_indicate_enabled = false;
		}
	}	
	else if (
		( p_evt_write->handle == p_wcs->target_indicate_handles.value_handle )
		&& (p_evt_write->len == 4)
	)
	{
		memcpy(m_target_profile_val, p_evt_write->data, 4);
	}
}

void on_indicate_comfirm( ble_wechat_t *p_wcs, ble_evt_t * p_ble_evt )
{
	p_wcs->indicate_comfirm_counter++;
}

void ble_wechat_on_ble_evt(ble_wechat_t *p_wcs, ble_evt_t * p_ble_evt ) 
{
	switch (p_ble_evt->header.evt_id)
    {
		case BLE_GAP_EVT_CONNECTED:
			p_wcs->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;	
			break;
        case BLE_GATTS_EVT_WRITE:
            on_write( p_wcs, p_ble_evt); 
            break;
        case BLE_GATTS_EVT_HVC:
			on_indicate_comfirm(p_wcs,p_ble_evt);
			break;	
		case BLE_GATTS_EVT_SYS_ATTR_MISSING:
			break;
		case BLE_GAP_EVT_DISCONNECTED:
			p_wcs->conn_handle = BLE_CONN_HANDLE_INVALID;
			break;
        default:
            break;
    }
}

//Add WeChat Service to GATTS sercvice
uint32_t ble_wechat_add_service(ble_wechat_t *p_wechat)
{
	uint32_t err_code;
	ble_uuid_t ble_wechat_uuid;
	BLE_UUID_BLE_ASSIGN(ble_wechat_uuid, BLE_UUID_WECHAT_SERVICE);
	err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_wechat_uuid, &p_wechat->service_handle);
	
	return err_code;
}

//Add the indicate characteristic 
uint32_t ble_wechat_add_indicate_char(ble_wechat_t *p_wechat)
{
	ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          char_uuid;
    ble_gatts_attr_md_t attr_md;
	
	memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
	cccd_md.vloc = BLE_GATTS_VLOC_STACK;
	
	memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.indicate = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;//&cccd_md;
    char_md.p_sccd_md         = NULL;
	
	BLE_UUID_BLE_ASSIGN(char_uuid, BLE_UUID_WECHAT_INDICATE_CHARACTERISTICS);
	memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
   
    attr_char_value.p_uuid       = &char_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(m_read_indicate_val);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = BLE_WECHAT_MAX_DATA_LEN;
    attr_char_value.p_value      = m_read_indicate_val;
    
    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->indicate_handles);
}

//Add the write characteristic 
static uint32_t ble_wechat_add_write_char(ble_wechat_t *p_wechat)
{
	ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          char_uuid;
    ble_gatts_attr_md_t attr_md;
		
	memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.write = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;
	
	BLE_UUID_BLE_ASSIGN(char_uuid, BLE_UUID_WECHAT_WRITE_CHARACTERISTICS);
	
	memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    
    attr_char_value.p_uuid       = &char_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(m_write_char_val);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = BLE_WECHAT_MAX_DATA_LEN;
    attr_char_value.p_value      = m_write_char_val;
    
    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->write_handles);
}

//Add the read characteristic 
static uint32_t ble_wechat_add_read_char(ble_wechat_t *p_wechat)
{
	ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          char_uuid;
    ble_gatts_attr_md_t attr_md;
	
	memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;
	
	BLE_UUID_BLE_ASSIGN(char_uuid, BLE_UUID_WECHAT_READ_CHARACTERISTICS);
	
	memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    
    attr_char_value.p_uuid       = &char_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(m_addl_adv_manuf_data)-2;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = BLE_WECHAT_MAX_DATA_LEN;
    attr_char_value.p_value      = m_addl_adv_manuf_data + 2;
    
    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->read_handles);
}

uint32_t wchat_pedometer_profile_init(ble_wechat_t *p_wechat)
{
	ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          char_uuid;
    ble_gatts_attr_md_t attr_md;
	
	m_pedometer_profile_val[0] = 0x1;
	
	memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
	cccd_md.vloc = BLE_GATTS_VLOC_STACK;
	
	memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.indicate = 1;
	char_md.char_props.read = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;//&cccd_md;
    char_md.p_sccd_md         = NULL;
	
	BLE_UUID_BLE_ASSIGN(char_uuid, BLE_UUID_WECHAT_PEDOMETER_CHARACTERISTICS);
	memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
   
    attr_char_value.p_uuid       = &char_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(m_pedometer_profile_val);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = BLE_WECHAT_MAX_DATA_LEN;
    attr_char_value.p_value      = m_pedometer_profile_val;
    
    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->pedometer_indicate_handles);	
}

uint32_t wchat_target_profile_init(ble_wechat_t *p_wechat)
{
	ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          char_uuid;
    ble_gatts_attr_md_t attr_md;
	
	m_target_profile_val[0] = 0x1;
	
	memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
	cccd_md.vloc = BLE_GATTS_VLOC_STACK;
	
	memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.indicate = 1;       // indicate ; read ; write 
	char_md.char_props.read = 1;
	char_md.char_props.write = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;//&cccd_md;
    char_md.p_sccd_md         = NULL;
	
	BLE_UUID_BLE_ASSIGN(char_uuid, BLE_UUID_WECHAT_TARGET_CHARACTERISTICS);
	memset(&attr_md, 0, sizeof(attr_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
   
    attr_char_value.p_uuid       = &char_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(m_target_profile_val);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = BLE_WECHAT_MAX_DATA_LEN;
    attr_char_value.p_value      = m_target_profile_val;
    
    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->target_indicate_handles);	
}

//Add the WeChat characteristic include indicate write and read characteristic
uint32_t ble_wechat_add_characteristics(ble_wechat_t *p_wechat)
{
	uint32_t err_code;
	
	err_code = ble_wechat_add_indicate_char(p_wechat);
	APP_ERROR_CHECK(err_code);
	
	err_code = ble_wechat_add_write_char(p_wechat);
	APP_ERROR_CHECK(err_code);
	
	err_code = ble_wechat_add_read_char(p_wechat);
	APP_ERROR_CHECK(err_code);
	
	return err_code;
}

static int ble_wechat_indicate(ble_wechat_t *p_wcs, unsigned char *buf, int length)
{
	ble_gatts_hvx_params_t hvx_params;
	unsigned short tx_length;
	
	if ( !p_wcs->pedometer_is_indicate_enabled )
	{
		return (-1);
	}
	
	memset(&hvx_params, 0, sizeof(hvx_params));
	tx_length = (unsigned short)length;
	
	hvx_params.handle = p_wcs->pedometer_indicate_handles.value_handle;
	hvx_params.p_data = buf;
	hvx_params.p_len  = &tx_length;
	hvx_params.type   = BLE_GATT_HVX_INDICATION;
	
	p_wcs->indicate_counter++;

	return sd_ble_gatts_hvx(p_wcs->conn_handle, &hvx_params);
}

int wechat_indicate_pedometer( unsigned long pedometer )
{
	unsigned char buf[4];
	
	buf[0] = 0x1;  // pedometer flag
	buf[1] = (pedometer>> 0) & 0xFF;
	buf[2] = (pedometer>> 8) & 0xFF;
	buf[3] = (pedometer>>16) & 0xFF;
	
	ble_wechat_indicate( &m_ble_wechat, buf, 4 );
	
	return 0;
}

void wechat_services_init(ble_wechat_t *ble_wechat)
{
	uint32_t err_code;
	err_code = ble_wechat_add_service(ble_wechat);
	APP_ERROR_CHECK(err_code);
	err_code = ble_wechat_add_characteristics(ble_wechat);
	APP_ERROR_CHECK(err_code);
	
	err_code = wchat_pedometer_profile_init(ble_wechat);
	APP_ERROR_CHECK(err_code);
	
	err_code = wchat_target_profile_init(ble_wechat);
	APP_ERROR_CHECK(err_code);
}






