
#include "OSAL.h"
#include "OSAL_Timer.h"
#include "OSAL_Task.h"
#include "Task_Ble.h"
#include "Task_Step.h"
#include "Task_GUI.h"
#include "Task_Motor.h"

#include "btprotocol.h"
#include "UserSetting.h"

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_nus.h"
#ifdef BLE_DFU_APP_SUPPORT
#include "ble_dfu.h"
#include "dfu_app_handler.h"
#endif // BLE_DFU_APP_SUPPORT
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "device_manager.h"
#include "pstorage.h"
#include "nrf_delay.h"
#include "ble_nus.h"

#if ( CONFIG_USING_ANCS == 1 )
#include "Ancs.h"
#include "ble_ancs_c.h"
#include "ble_db_discovery.h"
#endif

#include "ble_wechat_service.h"
#include "Task_Upload.h"


unsigned long taskBleTaskId;

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#define IS_SRVC_CHANGED_CHARACT_PRESENT  1                                          /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define CENTRAL_LINK_COUNT               0                                          /**< Number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT            1                                          /**< Number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define DEVICE_NAME                      "N108S"                                    /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                "NordicSemiconductor"                      /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                MSEC_TO_UNITS(250, UNIT_0_625_MS)           /**< The advertising interval (in units of 0.625 ms. This value corresponds to 25 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS       0                                          /**< The advertising timeout in units of seconds. */


#define SENSOR_CONTACT_DETECTED_INTERVAL APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER) /**< Sensor Contact Detected toggle interval (ticks). */

#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(80, UNIT_1_25_MS)           /**< Minimum acceptable connection interval (0.4 seconds). */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(100, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (0.65 second). */
#define SLAVE_LATENCY                    1                                          /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(4000, UNIT_10_MS)            /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  5000                                        /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   30000                                       /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                          /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                          /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                         /**< Maximum encryption key size. */

#define DEAD_BEEF                        0xDEADBEEF                                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#ifdef BLE_DFU_APP_SUPPORT
#define DFU_REV_MAJOR                    0x00                                       /** DFU Major revision number to be exposed. */
#define DFU_REV_MINOR                    0x01                                       /** DFU Minor revision number to be exposed. */
#define DFU_REVISION                     ((DFU_REV_MAJOR << 8) | DFU_REV_MINOR)     /** DFU Revision number to be exposed. Combined of major and minor versions. */
#define APP_SERVICE_HANDLE_START         0x000C                                     /**< Handle of first application specific service when when service changed characteristic is present. */
#define BLE_HANDLE_MAX                   0xFFFF                                     /**< Max handle value in BLE. */

STATIC_ASSERT(IS_SRVC_CHANGED_CHARACT_PRESENT);                                     /** When having DFU Service support in application the Service Changed Characteristic should always be present. */
#endif // BLE_DFU_APP_SUPPORT


static uint16_t                          m_conn_handle = BLE_CONN_HANDLE_INVALID;   /**< Handle of the current connection. */


static dm_application_instance_t         m_app_handle;                              /**< Application identifier allocated by device manager */

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE},
                                   {BLE_UUID_WECHAT_SERVICE, BLE_UUID_TYPE_BLE}
																				 }; /**< Universally unique service identifiers. */

static ble_nus_t                         m_nus;                                     /**< Structure to identify the Nordic UART Service. */
#ifdef BLE_DFU_APP_SUPPORT
static ble_dfu_t                         m_dfus;                                    /**< Structure used to identify the DFU service. */
#endif // BLE_DFU_APP_SUPPORT
								   
unsigned char rxbuf[20];

int ble_write_bytes( unsigned char *buf, int length )
{
	return ble_nus_string_send( &m_nus, buf, length);
}

/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
	for ( unsigned int i = 0; i < length; i++ )
	{
		rxbuf[i] = p_data[i];
	}
	
	bt_protocol_rx( p_data, length );
}
/**@snippet [Handling the data received over BLE] */
								   
								   

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


#ifdef BLE_DFU_APP_SUPPORT
/**@brief Function for stopping advertising.
 */
int adv_stop_try_cnt = 0;

static void advertising_stop(void)
{
    uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    
    // 2018.05.25
    
    if ( err_code != NRF_SUCCESS )
    {
        adv_stop_try_cnt++;
    
        if ( adv_stop_try_cnt < 10 )
        {
            osal_start_timerEx( taskBleTaskId, TASK_BLE_TURN_OFF_ADV_EVT, 100);
        }else{
            if ( config.shutdown == 0 )
            {
                APP_ERROR_CHECK(err_code);
            }
        }
    }else{
        adv_stop_try_cnt = 0;
    }
}

/**@brief Function for loading application-specific context after establishing a secure connection.
 *
 * @details This function will load the application context and check if the ATT table is marked as
 *          changed. If the ATT table is marked as changed, a Service Changed Indication
 *          is sent to the peer if the Service Changed CCCD is set to indicate.
 *
 * @param[in] p_handle The Device Manager handle that identifies the connection for which the context
 *                     should be loaded.
 */
static void app_context_load(dm_handle_t const * p_handle)
{
    uint32_t                 err_code;
    static uint32_t          context_data;
    dm_application_context_t context;

    context.len    = sizeof(context_data);
    context.p_data = (uint8_t *)&context_data;

    err_code = dm_application_context_get(p_handle, &context);
    if (err_code == NRF_SUCCESS)
    {
        // Send Service Changed Indication if ATT table has changed.
        if ((context_data & (DFU_APP_ATT_TABLE_CHANGED << DFU_APP_ATT_TABLE_POS)) != 0)
        {
            err_code = sd_ble_gatts_service_changed(m_conn_handle, APP_SERVICE_HANDLE_START, BLE_HANDLE_MAX);
            if ((err_code != NRF_SUCCESS) &&
                (err_code != BLE_ERROR_INVALID_CONN_HANDLE) &&
                (err_code != NRF_ERROR_INVALID_STATE) &&
                (err_code != BLE_ERROR_NO_TX_PACKETS) &&
                (err_code != NRF_ERROR_BUSY) &&
                (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING))
            {
                APP_ERROR_HANDLER(err_code);
            }
        }

        err_code = dm_application_context_delete(p_handle);
        APP_ERROR_CHECK(err_code);
    }
    else if (err_code == DM_NO_APP_CONTEXT)
    {
        // No context available. Ignore.
    }
    else
    {
        APP_ERROR_HANDLER(err_code);
    }
}


/** @snippet [DFU BLE Reset prepare] */
/**@brief Function for preparing for system reset.
 *
 * @details This function implements @ref dfu_app_reset_prepare_t. It will be called by
 *          @ref dfu_app_handler.c before entering the bootloader/DFU.
 *          This allows the current running application to shut down gracefully.
 */
static void reset_prepare(void)
{
    uint32_t err_code;

    if (m_conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        // Disconnect from peer.
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
    }
    else
    {
        // If not connected, the device will be advertising. Hence stop the advertising.
        advertising_stop();
    }

    err_code = ble_conn_params_stop();
    APP_ERROR_CHECK(err_code);

    nrf_delay_ms(500);
}
/** @snippet [DFU BLE Reset prepare] */
#endif // BLE_DFU_APP_SUPPORT

#include "Log.h"

void ConnectionTerminate( int enable )
{
    uint32_t err_code;
	
	if ( enable == 1 )
	{
		if ( m_conn_handle != BLE_CONN_HANDLE_INVALID )
		{
			// Disconnect from peer.
			err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
			APP_ERROR_CHECK(err_code);
		}

		// turn the advertising off after 5 seconds.
		// must to do this (delay), otherwise ble stack will
		// return error code.
		osal_start_timerEx( taskBleTaskId, TASK_BLE_TURN_OFF_ADV_EVT, 500);
		
	}else{
		ble_advertising_start(BLE_ADV_MODE_SLOW);
	}
}

/**@brief Function for initializing services that will be used by the application.
 *
 * @details Initialize the Heart Rate, Battery and Device Information services.
 */
static void services_init(void)
{
    uint32_t       err_code;
	
    ble_nus_init_t nus_init;
    
	// Initialize Nordic uart Service.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;
    
    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);	
	
#ifdef BLE_DFU_APP_SUPPORT
    /** @snippet [DFU BLE Service initialization] */
    ble_dfu_init_t   dfus_init;

    // Initialize the Device Firmware Update Service.
    memset(&dfus_init, 0, sizeof(dfus_init));

    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.error_handler = NULL;
    dfus_init.evt_handler   = dfu_app_on_dfu_evt;
    dfus_init.revision      = DFU_REVISION;

    err_code = ble_dfu_init(&m_dfus, &dfus_init);
    APP_ERROR_CHECK(err_code);

    dfu_app_reset_prepare_set(reset_prepare);
    dfu_app_dm_appl_instance_set(m_app_handle);
    /** @snippet [DFU BLE Service initialization] */
#endif // BLE_DFU_APP_SUPPORT

#if ( CONFIG_USING_WECHAT == 1)
	//** add for weixin */
	
	wechat_services_init(&m_ble_wechat);
	// end add
#endif

#if ( CONFIG_USING_ANCS == 1 )
	ancs_service_init();
#endif
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code;

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
            {
        case BLE_GAP_EVT_CONNECTED:
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
			osal_set_event ( taskBleTaskId, TASK_BLE_CONNECTED_EVT );
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
			osal_set_event ( taskBleTaskId, TASK_BLE_DISCONNECTED_EVT);
            break;		
		case BLE_GAP_EVT_TIMEOUT:
			m_conn_handle = BLE_CONN_HANDLE_INVALID;
			osal_set_event ( taskBleTaskId, TASK_BLE_TIMEOUT_EVT );
			break;		
        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
	ble_nus_on_ble_evt(&m_nus, p_ble_evt);
#ifdef BLE_DFU_APP_SUPPORT
    /** @snippet [Propagating BLE Stack events to DFU Service] */
    ble_dfu_on_ble_evt(&m_dfus, p_ble_evt);
    /** @snippet [Propagating BLE Stack events to DFU Service] */
#endif // BLE_DFU_APP_SUPPORT
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
	
#if ( CONFIG_USING_WECHAT == 1)
	ble_wechat_on_ble_evt( &m_ble_wechat, p_ble_evt );
#endif
	
#if ( CONFIG_USING_ANCS == 1 )
	ble_ancs_c_on_ble_evt(&m_ancs_c, p_ble_evt);
	ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
#endif	
}

/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    
    // Initialize the SoftDevice handler module.	
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

#ifdef BLE_DFU_APP_SUPPORT
    ble_enable_params.gatts_enable_params.service_changed = 1;
	ble_enable_params.common_enable_params.vs_uuid_count  = 2;
#endif // BLE_DFU_APP_SUPPORT
	
#if ((CONFIG_USING_ANCS == 1) && (BLE_DFU_APP_SUPPORT == 1))
	ble_enable_params.common_enable_params.vs_uuid_count += 4;
#endif	
#if ((CONFIG_USING_ANCS != 1) && (BLE_DFU_APP_SUPPORT != 1))
#error "Please modify here (vs_uuid_count) "
#endif

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);

    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_event,
                                           ret_code_t        event_result)
{
	// remove flow code 2017.6.28
    // APP_ERROR_CHECK(event_result);
	
#ifdef BLE_DFU_APP_SUPPORT
    if (p_event->event_id == DM_EVT_LINK_SECURED)
    {
        app_context_load(p_handle);
    }
#endif // BLE_DFU_APP_SUPPORT

#if (CONFIG_USING_ANCS == 1)
	uint32_t err_code;
	dm_security_status_t status;
	
    switch (p_event->event_id)
    {
        case DM_EVT_CONNECTION:
            m_peer_handle = (*p_handle);
#if 0       /* Make a security link request after connecting directly */
			osal_start_timerEx( taskBleTaskId, TASK_BLE_SECURITY_EVT, 2000);
#endif
            break;
		case DM_EVT_SECURITY_SETUP_COMPLETE:
			err_code = dm_security_status_req(&m_peer_handle, &status);
			APP_ERROR_CHECK(err_code);

			// If the link is still not secured by the peer, initiate security procedure.
			if (status == NOT_ENCRYPTED)
			{
				osal_set_event ( taskBleTaskId, TASK_BLE_PAIRED_FAILED_EVT);
			}else{
                osal_set_event ( taskBleTaskId, TASK_BLE_PAIRED_OK_EVT);
			}				
			break;

        case DM_EVT_LINK_SECURED:
		#if 0
            err_code = ble_db_discovery_start(&m_ble_db_discovery,
                                              p_event->event_param.p_gap_param->conn_handle);
            APP_ERROR_CHECK(err_code);                                              p_event->event_param.p_gap_param->conn_handle);
		#else
			osal_start_timerEx ( taskBleTaskId, ANCS_DISCOVERY_DELAY_START_EVT, 20000);		
		#endif
            break; 

        default:
            break;
    }
#endif	

    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.lesc         = SEC_PARAM_LESC;
    register_param.sec_param.keypress     = SEC_PARAM_KEYPRESS;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}

extern void gain_mac( unsigned char *buffer);

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = false;
    advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_slow_enabled  = BLE_ADV_SLOW_ENABLED;
    options.ble_adv_slow_interval = APP_ADV_INTERVAL;
    options.ble_adv_slow_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;
	
#if ( CONFIG_USING_WECHAT == 1)
	/** add for weixin */
	
	m_addl_adv_manuf_data[0] = 0xFE;
	m_addl_adv_manuf_data[1] = 0xE7;
	gain_mac( m_addl_adv_manuf_data + 2 );
	
	ble_advdata_manuf_data_t        manuf_data;
    manuf_data.company_identifier = COMPANY_IDENTIFIER;
    manuf_data.data.size          = sizeof(m_addl_adv_manuf_data);
    manuf_data.data.p_data        = m_addl_adv_manuf_data;
    advdata.p_manuf_specific_data = &manuf_data;

	/** end */
#endif

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Power manager.
 */
void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

#include "Log.h"
#include "Task_Flash.h"
#include "Window.h"
#include "wdt.h"
volatile unsigned long dog_is_shutdown_cnt = 0;
long ret_code = 0;

unsigned long taskBle( unsigned long task_id, unsigned long events )
{
	if ( events & TASK_BLE_WDT_FEED_EVT )
	{
		#if (MY_WDT_CONFIG_ENABLE==1)
		if ( (bool)NRF_WDT->RUNSTATUS == false)
		{
			WDT_Init();
			dog_is_shutdown_cnt++;
		}			
		WDT_Feed();
		#endif
		
		return ( events ^ TASK_BLE_WDT_FEED_EVT );
	}
	
	if ( events & TASK_BLE_WC_PEDOMETER_EVT )
	{
		osal_start_timerEx( task_id, TASK_BLE_WC_PEDOMETER_EVT, 10000 );
		
		wechat_indicate_pedometer( pedometer.counter );
		
		return ( events ^ TASK_BLE_WC_PEDOMETER_EVT );
	}
	
    if ( events & TASK_BLE_SYSTEM_OFF_EVT )
    {
        /**
 		 * Go to system-off mode (this function will not return;
		 * wakeup will cause a reset).
		 */
		sleep_mode_enter();	
        
        return ( events ^ TASK_BLE_SYSTEM_OFF_EVT );
    }
	
	if ( events & TASK_BLE_SYSTEM_REBOOT_EVT )
	{
		NVIC_SystemReset();
		
		return ( events ^ TASK_BLE_SYSTEM_REBOOT_EVT );
	}

	if ( events & TASK_BLE_CONNECTED_EVT )
	{
#if 0
		/** ÇÄ»Ï¨Ä°×´Ì¬Ü²ÖŸÝ†Ò½Þ§Ä¦Ð”Ê¾×¾Î¬Ã«Ê±à ‰Ó” TOUCH */
		if ( config.lcdEnable == 0 || win_current == 1 )
		{
			osal_set_event( taskGUITaskId, TASK_GUI_TOUCH_EVT );
		}
#endif		
		// Dont' using this function
		// osal_set_event( taskMotorTaskId, TASK_MOTOR_BLE_CONNECT_EVT );
		
		ble.isConnected = 1;
		config.hrWaveUploadEnable = 0;			
		
		return ( events ^ TASK_BLE_CONNECTED_EVT );
	}
	
	if ( events & TASK_BLE_TIMEOUT_EVT )
	{
		ble.isConnected = 0;
		config.hrWaveUploadEnable = 0;
		config.gsensor_debug = 0;
		
		osal_set_event ( taskUploadTaskId, TASK_UPLOAD_RESET_QUEUE_EVT);
		
		// in sleep mode ble event no handle

		if ( config.notify_enable != 1 || config.sleep_mode != 0 
			|| config.shutdown != 0 )
		{
			return ( events ^ TASK_BLE_TIMEOUT_EVT );
		}		
		
		if ( config.delay_timeout == 0 ){
			osal_set_event ( taskGUITaskId, TASK_GUI_BLE_DISCONNECT_EVT );
		}else{
			osal_start_timerEx( taskGUITaskId, TASK_GUI_BLE_DISCONNECT_EVT, 
				config.delay_timeout * 1000);
		}
		
		return ( events ^ TASK_BLE_TIMEOUT_EVT );
	}
	
	if ( events & TASK_BLE_DISCONNECTED_EVT )
	{
		ble.isConnected = 0;
		config.gsensor_debug = 0;
		config.hrWaveUploadEnable = 0;

		osal_set_event ( taskUploadTaskId, TASK_UPLOAD_RESET_QUEUE_EVT);

		// in sleep mode ble event no handle
		
		if ( config.notify_enable != 1 || config.sleep_mode != 0 
			|| config.shutdown != 0 )
		{
			return ( events ^ TASK_BLE_DISCONNECTED_EVT );
		}
		
		if ( config.delay_timeout == 0 ){
			osal_set_event ( taskGUITaskId, TASK_GUI_BLE_DISCONNECT_EVT );
		}else{
			osal_start_timerEx( taskGUITaskId, TASK_GUI_BLE_DISCONNECT_EVT, 
				config.delay_timeout * 1000);
		}	
		
		return ( events ^ TASK_BLE_DISCONNECTED_EVT );
	}
	
	if ( events & TASK_BLE_SECURITY_EVT )
	{
#if (CONFIG_USING_ANCS == 1)
		slave_security_request();
#endif
		return ( events ^ TASK_BLE_SECURITY_EVT );
	}
	
	if ( events & TASK_BLE_TURN_OFF_ADV_EVT )
	{		
		advertising_stop();		
			
		return ( events ^ TASK_BLE_TURN_OFF_ADV_EVT );
	}
	
	if ( events & TASK_BLE_PAIRED_FAILED_EVT )
	{
		unsigned char cmd[20];
		
		cmd[0] = 0x88;
		cmd[1] = 0xF0;
		cmd[2] = 0x01;		// ·Ç0ÔòÊ§°Ü
			
		bt_protocol_tx( cmd, 20 );
		
		return ( events ^ TASK_BLE_PAIRED_FAILED_EVT ); 
	}
	
	if ( events & TASK_BLE_PAIRED_OK_EVT )
	{
		unsigned char cmd[20];
			
		cmd[0] = 0x88;
		cmd[1] = 0xF0;
		cmd[2] = 0x00;     // ·Ç0ÔòÊ§°Ü
		
		bt_protocol_tx( cmd, 20 );
			
		return ( events ^ TASK_BLE_PAIRED_OK_EVT );
	}
    
	if ( events & TASK_BLE_INIT_EVT )
	{ 		
		uint32_t err_code;
		
		// osal_start_timerEx( task_id, TASK_BLE_SYSTEM_OFF_EVT, 10000 );
        
		err_code = ble_advertising_start(BLE_ADV_MODE_SLOW);
		APP_ERROR_CHECK(err_code);
		
		osal_start_timerEx( task_id, TASK_BLE_WC_PEDOMETER_EVT, 10000 );
		
		///////////
		#if (MY_WDT_CONFIG_ENABLE==1)
		if ( (bool)NRF_WDT->RUNSTATUS == false)
		{
			WDT_Init();
			dog_is_shutdown_cnt++;
		}
		WDT_Feed();	
		#endif		
		        
		return ( events ^ TASK_BLE_INIT_EVT );
	}
	
	if ( events & ANCS_DISCOVERY_DELAY_START_EVT )
	{
		memset(&m_ble_db_discovery,0,sizeof(m_ble_db_discovery));
		
		ret_code = ble_db_discovery_start(&m_ble_db_discovery, m_peer_handle.connection_id);
		
		return ( events ^ ANCS_DISCOVERY_DELAY_START_EVT );
	}	

	return ( 0 );
}

unsigned long taskBleTaskId;

void taskBleInit( unsigned long task_id )
{
	taskBleTaskId = task_id;
	
	/** To initialize the BLE stack */
	{		
		bool erase_bonds;

		// Initialize.
		ble_stack_init();		
		device_manager_init(erase_bonds);
#if (CONFIG_USING_ANCS == 1)		
		db_discovery_init();
#endif
		gap_params_init();
		advertising_init();
		services_init();
		conn_params_init();		
	}
	
    osal_set_event(task_id, TASK_BLE_INIT_EVT);
}
