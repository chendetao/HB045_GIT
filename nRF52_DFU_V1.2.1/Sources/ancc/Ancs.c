#include "Ancs.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "nrf_delay.h"
#include "ble_ancs_c.h"
#include "ble_db_discovery.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble_hci.h"
#include "ble_gap.h"
#include "device_manager.h"
#include "pstorage.h"
#include "nrf_soc.h"
#include "softdevice_handler.h"
#include "nrf_log.h"

#include "OSAL.h"
#include "Task_Ble.h"
#include "MsgQueue.h"
#include "UI_win_notify.h"
#include "U2K.h"
#include "UserSetting.h"

#define ATTR_DATA_SIZE                  BLE_ANCS_ATTR_DATA_MAX                      /**< Allocated size for attribute data. */


/**@brief String literals for the iOS notification categories. used then printing to UART. */

/* static const char * lit_catid[BLE_ANCS_NB_OF_CATEGORY_ID] =
{
    "Other",
    "Incoming Call",
    "Missed Call",
    "Voice Mail",
    "Social",
    "Schedule",
    "Email",
    "News",
    "Health And Fitness",
    "Business And Finance",
    "Location",
    "Entertainment"
}; */

/**@brief String literals for the iOS notification event types. used then printing to UART. */

/* static const char * lit_eventid[BLE_ANCS_NB_OF_EVT_ID] =
{
    "Added",
    "Modified",
    "Removed"
}; */

/**@brief String literals for the iOS notification attribute types. used then printing to UART. */

/* static const char * lit_attrid[BLE_ANCS_NB_OF_ATTRS] =
{
    "App Identifier",
    "Title",
    "Subtitle",
    "Message",
    "Message Size",
    "Date",
    "Positive Action Label",
    "Negative Action Label"
}; */

ble_ancs_c_t              m_ancs_c;                                 	   /**< Structure used to identify the Apple Notification Service Client. */
ble_db_discovery_t        m_ble_db_discovery;                       /**< Structure used to identify the DB Discovery module. */
static ble_ancs_c_evt_notif_t m_notification_latest;                       /**< Local copy to keep track of the newest arriving notifications. */
dm_handle_t               m_peer_handle;                                   /**< Identifies the peer that is currently connected. */


static uint8_t m_attr_id[ATTR_DATA_SIZE];
static uint8_t m_attr_title[ATTR_DATA_SIZE];                               /**< Buffer to store attribute data. */
// static uint8_t m_attr_subtitle[ATTR_DATA_SIZE];                            /**< Buffer to store attribute data. */
static uint8_t m_attr_message[ATTR_DATA_SIZE];                             /**< Buffer to store attribute data. */
// static uint8_t m_attr_message_size[ATTR_DATA_SIZE];                        /**< Buffer to store attribute data. */
// static uint8_t m_attr_date[ATTR_DATA_SIZE];                                /**< Buffer to store attribute data. */
// static uint8_t m_attr_posaction[ATTR_DATA_SIZE];                           /**< Buffer to store attribute data. */
// static uint8_t m_attr_negaction[ATTR_DATA_SIZE];                           /**< Buffer to store attribute data. */

/**@brief Function for handling the security request timer time-out.
 *
 * @details This function is called each time the security request timer expires.
 *
 * @param[in] p_context  Pointer used for passing context information from the
 *                       app_start_timer() call to the time-out handler.
 */
int slave_security_request(void)
{
    uint32_t             err_code;
    dm_security_status_t status;

    if ( m_peer_handle.connection_id != DM_INVALID_ID )
    {
        err_code = dm_security_status_req(&m_peer_handle, &status);
        APP_ERROR_CHECK(err_code);

        // If the link is still not secured by the peer, initiate security procedure.
        if (status == NOT_ENCRYPTED)
        {
            err_code = dm_security_setup_req(&m_peer_handle);
            APP_ERROR_CHECK(err_code);
			
			return 0;
        }else{
			osal_set_event( taskBleTaskId, TASK_BLE_PAIRED_OK_EVT);
		}
    }
	
	return 1;
}

/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
static void apple_notification_setup(void)
{
    uint32_t err_code;

    nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    err_code = ble_ancs_c_notif_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(err_code);

    err_code = ble_ancs_c_data_source_notif_enable(&m_ancs_c);
    APP_ERROR_CHECK(err_code);

    NRF_LOG("Notifications Enabled.\r\n");
}

extern void notify_remove(void);
unsigned char last_call_uuid[4];

/**@brief Function for printing an iOS notification.
 *
 * @param[in] p_notif  Pointer to the iOS notification.
 */
static void notif_print(ble_ancs_c_evt_notif_t * p_notif)
{
    NRF_LOG("\r\nNotification\r\n");
    NRF_LOG_PRINTF("Event:       %s\r\n", lit_eventid[p_notif->evt_id]);
    NRF_LOG_PRINTF("Category ID: %s\r\n", lit_catid[p_notif->category_id]);
    NRF_LOG_PRINTF("Category Cnt:%u\r\n", (unsigned int) p_notif->category_count);
    NRF_LOG_PRINTF("UID:         %u\r\n", (unsigned int) p_notif->notif_uid);

    NRF_LOG("Flags:\r\n");
    if(p_notif->evt_flags.silent == 1)
    {
        NRF_LOG(" Silent\r\n");
    }
    if(p_notif->evt_flags.important == 1)
    {
        NRF_LOG(" Important\r\n");
    }
    if(p_notif->evt_flags.pre_existing == 1)
    {
        NRF_LOG(" Pre-existing\r\n");
    }
    if(p_notif->evt_flags.positive_action == 1)
    {
        NRF_LOG(" Positive Action\r\n");
    }
    if(p_notif->evt_flags.negative_action == 1)
    {
        NRF_LOG(" Positive Action\r\n");
    }

	if ( p_notif->evt_id == 0 && 
		p_notif->category_id != BLE_ANCS_CATEGORY_ID_MISSED_CALL )
	{
		// Add
		ble_ancs_c_request_attrs( &m_ancs_c,  p_notif );
	}else if ( p_notif->evt_id == 2 )
	{
		// Removed
		notify_remove();
	}
    
    ancs_ancs_flag = 1;    
}

static int is_Emoji( unsigned long code )
{
	if ( ((code >= 0x1F601) && (code <= 0x1F64F))
		|| ((code >= 0x2702) && (code <= 0x27B0)) )
	{		
		return 1;
	} else {
		return 0;
	}
}

unsigned char subtitle[128] = {0};

/**@brief Function for printing iOS notification attribute data.
 * 
 * @param[in] p_attr           Pointer to an iOS notification attribute.
 * @param[in] p_ancs_attr_list Pointer to a list of attributes. Each entry in the list stores 
                               a pointer to its attribute data, which is to be printed.
 */
static void notif_attr_print(ble_ancs_c_evt_notif_attr_t * p_attr)
{
	static unsigned char nid = 0;
	
    if (p_attr->attr_len != 0)
    {
        NRF_LOG_PRINTF("%s: %s\r\n", lit_attrid[p_attr->attr_id], p_attr->p_attr_data);
    }
    else if (p_attr->attr_len == 0)
    {
        NRF_LOG_PRINTF("%s: (N/A)\r\n", lit_attrid[p_attr->attr_id]);
    }
	
	if ( p_attr->attr_id == BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE )
	{
		 memcpy(subtitle,(char*)p_attr->p_attr_data, p_attr->attr_len ); 
	}
	
	if ( p_attr->attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER )
	{
		 if (!strncmp((char*)p_attr->p_attr_data, "com.apple.mobilephone", 21))
		 {
			mq.type = NOTIFY_TYPE_CALL; 
			nid = NOFIFY_ID_CALLING;
			 
			last_call_uuid[3] = (p_attr->notif_uid>>24)&0xFF;
			last_call_uuid[2] = (p_attr->notif_uid>>16)&0xFF;
			last_call_uuid[1] = (p_attr->notif_uid>> 8)&0xFF;
			last_call_uuid[0] = (p_attr->notif_uid>> 0)&0xFF;
			 
		 }else if (!strncmp((char*)p_attr->p_attr_data, "com.apple.MobileSMS", 19))
		 {
			mq.type = NOTIFY_TYPE_MSG;
			nid = NOTIFY_ID_EMAIL;
		 }else if (!strncmp((char*)p_attr->p_attr_data, "com.tencent.xin", 15))
		 {
			mq.type = NOTIFY_TYPE_WX;
			nid = NOTIFY_ID_APP_MSG;
		 }else if ( !strncmp((char*)p_attr->p_attr_data, "com.tencent.mqq", 15)
			|| !strncmp((char*)p_attr->p_attr_data, "com.tencent.mQQi", 16) )
		 {
			mq.type = NOTIFY_TYPE_QQ;
			nid = NOTIFY_ID_APP_MSG;
		 }else if ( !strncmp((char*)p_attr->p_attr_data, "com.facebook.Facebook",21))
		 {
			mq.type = NOTIFY_TYPE_FACEBOOK;
			nid = NOTIFY_ID_APP_MSG;
		 }else if ( !strncmp((char*)p_attr->p_attr_data, "net.whatsapp.WhatsApp",21))
		 {
			mq.type = NOTIFY_TYPE_WHATSAPP;
			mq.push_by_ios = 1;
			nid = NOTIFY_ID_APP_MSG;
		 }else if ( !strncmp((char*)p_attr->p_attr_data, "jp.naver.line",13) )
		 {
			mq.type = NOTIFY_TYPE_LINE;
			nid = NOTIFY_ID_APP_MSG;	
            mq.push_by_ios = 1;             
		 }else if ( !strncmp((char*)p_attr->p_attr_data, "com.skype.skype",15) )
		 {
			mq.type = NOTIFY_TYPE_SKYPE;
			nid = NOTIFY_ID_APP_MSG;			 
		 }else if ( !strncmp((char*)p_attr->p_attr_data, "com.atebits.Tweetie2",20) )
		 {
			mq.type = NOTIFY_TYPE_TT;
			nid = NOTIFY_ID_APP_MSG;			 
		 }else if ( !strncmp((char*)p_attr->p_attr_data, "com.burbn.instagram",19) )
		 {
			mq.type = NOTIFY_TYPE_INS;
			nid = NOTIFY_ID_APP_MSG;			 
		 }
		 else if ( config.app_all_enable == 1) 
         {
            mq.type = NOTIFY_TYPE_ALL;
            nid = NOTIFY_ID_APP_MSG;
         }else
		 {
			mq.type = NOTIFY_TYPE_INVALID;
		 }
	}
	
	if ( p_attr->attr_id == BLE_ANCS_NOTIF_ATTR_ID_TITLE )
	{
		const unsigned char *p = p_attr->p_attr_data;
		int i = 0, j = 0;
		unsigned long unic;
		unsigned char unicode_buf[2];
		
		switch( mq.type )
		{
		case NOTIFY_TYPE_WHATSAPP:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_WHATAPP) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_FACEBOOK:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_FACEBOOK) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_CALL:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_CALL) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_MSG:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_SMS) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_QQ:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_QQ) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_WX:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_WX) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_LINE:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_LINE) )
			{
				return;
			}			
			break;	
		case NOTIFY_TYPE_TT:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_TT) )
			{
				return;
			}			
			break;
		case NOTIFY_TYPE_SKYPE:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_SKYPE) )
			{
				return;
			}			
			break;
		case NOTIFY_TYPE_INS:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_INS) )
			{
				return;
			}			
			break;            
        case NOTIFY_TYPE_ALL:
            if ( config.app_all_enable == 0 )
            {
                return;
            }
		default:
			return;
		}		
		
		MessageClearAll();
		/** 
		 * title_valid = 0 default title "wechat"
		 * title_valid = 1 "for new iOS version"
		 */
		int title_valid = strncmp((char*)p_attr->p_attr_data, "WeChat",6);
		
		while( i < p_attr->attr_len)
		{
		  enc_utf8_to_unicode_one(p+j, &unic);
		  
		  i += enc_get_utf8_size(p+j);
		  unicode_buf[0] = unic>>8;
		  unicode_buf[1] = unic&0xFF;
		  j = i;   

		  if ( (mq.type == NOTIFY_TYPE_WX) && (title_valid != 0) )
		  {
			  MessageBuildContent( p_attr->notif_uid, 2, unicode_buf);			  			
		  }else{
			  MessageBuildSender( p_attr->notif_uid, 2, unicode_buf); 
		  }
		}	
		if ( (mq.type == NOTIFY_TYPE_WX) && (title_valid != 0) )
		{
		  // For wechat new version on iOS system.
		  unicode_buf[0] = 0;
		  unicode_buf[1] = ':';
		  MessageBuildContent( p_attr->notif_uid, 2, unicode_buf);
		} 	
	}
	
	if ( p_attr->attr_id == BLE_ANCS_NOTIF_ATTR_ID_MESSAGE )
	{
		const unsigned char *p = p_attr->p_attr_data;
		int i = 0, j = 0;
		unsigned long unic;
		unsigned char unicode_buf[16];
		
		switch( mq.type )
		{
		case NOTIFY_TYPE_WHATSAPP:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_WHATAPP) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_FACEBOOK:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_FACEBOOK) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_CALL:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_CALL) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_MSG:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_SMS) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_QQ:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_QQ) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_WX:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_WX) )
			{
				return;
			}
			break;
		case NOTIFY_TYPE_LINE:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_LINE) )
			{
				return;
			}			
			break;	
		case NOTIFY_TYPE_TT:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_TT) )
			{
				return;
			}			
			break;
		case NOTIFY_TYPE_SKYPE:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_SKYPE) )
			{
				return;
			}			
			break;
		case NOTIFY_TYPE_INS:
			if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_INS) )
			{
				return;
			}			
			break;            
        case NOTIFY_TYPE_ALL:
            if ( config.app_all_enable == 0 )
            {
                return;
            }            
		default:
			return;
		}
		
		if ( !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_WHATAPP )	
			&& !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_FACEBOOK )
			&& !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_CALL )
			&& !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_SMS )
			&& !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_QQ )
			&& !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_WX )
			&& !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_LINE ) 
			&& !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_TT ) 
			&& !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_SKYPE ) 
			&& !(ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_INS )        
            && config.app_all_enable == 0 )
		{
			return;
		}
		
		while( i < p_attr->attr_len)
		{
		  enc_utf8_to_unicode_one(p+j, &unic);

		  i += enc_get_utf8_size(p+j);
		  j = i;
			
		  if ( is_Emoji( unic ) ){  /** Emoji will be replace by "[emoji]" */
			  
			unicode_buf[0] = 0;
			unicode_buf[1] = '[';			  
			unicode_buf[2] = 0;
			unicode_buf[3] = 'e';			  
			unicode_buf[4] = 0;
			unicode_buf[5] = 'm';			  			  
			unicode_buf[6] = 0;
			unicode_buf[7] = 'o';			  	
			unicode_buf[8] = 0;
			unicode_buf[9] = 'j';		
			unicode_buf[10] = 0;
			unicode_buf[11] = 'i';	
			unicode_buf[12] = 0;
			unicode_buf[13] = ']';			  
			MessageBuildContent( p_attr->notif_uid, 14, unicode_buf);
			  
		  }else{
			unicode_buf[0] = unic>>8;
			unicode_buf[1] = unic&0xFF;
			  
			MessageBuildContent( p_attr->notif_uid, 2, unicode_buf);
		  }			  
		}
		
		int enable = 0;
		switch( mq.type )
		{
		case NOTIFY_TYPE_WHATSAPP:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_WHATAPP )
			{
				enable = 1;
			}
			break;
		case NOTIFY_TYPE_FACEBOOK:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_FACEBOOK )
			{
				enable = 1;
			}
			break;
		case NOTIFY_TYPE_CALL:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_CALL )
			{
				enable = 1;
			}
			break;
		case NOTIFY_TYPE_MSG:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_SMS )
			{
				enable = 1;
			}
			break;
		case NOTIFY_TYPE_QQ:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_QQ )
			{
				enable = 1;
			}
			break;
		case NOTIFY_TYPE_WX:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_WX )
			{
				enable = 1;
			}
			break;
		case NOTIFY_TYPE_LINE:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_LINE )
			{
				enable = 1;
			}			
			break;	
		case NOTIFY_TYPE_TT:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_TT )
			{
				enable = 1;
			}			
			break;
		case NOTIFY_TYPE_SKYPE:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_SKYPE )
			{
				enable = 1;
			}			
			break;
		case NOTIFY_TYPE_INS:
			if ( ble.ancs_notify_enable & ANCS_NOTIFY_ENABLE_INS )
			{
				enable = 1;
			}			
			break;            
        case NOTIFY_TYPE_ALL:
            if ( config.app_all_enable == 1 )
            {
                enable = 1;
            }
            break;
		default:
			break;
		}
		if ( enable == 1 )
		{
			NOTICE_ADD(nid);
		}
	}
}

bool ancs_perform_notification_action( bool action )
{	
	return ble_ancs_perform_notification_action(&m_ancs_c, last_call_uuid, false);
}

/**@brief Function for handling the Apple Notification Service client.
 *
 * @details This function is called for all events in the Apple Notification client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Notification Service client.
 */
static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_evt->evt_type)
    {
        case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
            NRF_LOG("Apple Notification Service discovered on the server.\r\n");
            err_code = ble_ancs_c_handles_assign(&m_ancs_c,p_evt->conn_handle, &p_evt->service);
            APP_ERROR_CHECK(err_code);
            apple_notification_setup();
            break;

        case BLE_ANCS_C_EVT_NOTIF:
            m_notification_latest = p_evt->notif;
            notif_print(&m_notification_latest);
            break;

        case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
            notif_attr_print(&p_evt->attr);
            break;

        case BLE_ANCS_C_EVT_DISCOVERY_FAILED:
            NRF_LOG("Apple Notification Service not discovered on the server.\r\n");
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling the Apple Notification Service client errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void apple_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Apple Notification Center Service.
*/
void ancs_service_init(void)
{
    ble_ancs_c_init_t ancs_init_obj;
    uint32_t          err_code;

    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));

	err_code = ble_ancs_c_attr_add(&m_ancs_c, 
								   BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER,
								   m_attr_id,
								   ATTR_DATA_SIZE);
	APP_ERROR_CHECK(err_code);
	
    err_code = ble_ancs_c_attr_add(&m_ancs_c,
                                   BLE_ANCS_NOTIF_ATTR_ID_TITLE,
                                   m_attr_title,
                                   ATTR_DATA_SIZE);
    APP_ERROR_CHECK(err_code);
    
    /* err_code = ble_ancs_c_attr_add(&m_ancs_c,
                                   BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE,
                                   m_attr_subtitle,
                                   ATTR_DATA_SIZE);
    APP_ERROR_CHECK(err_code);
	*/

    err_code = ble_ancs_c_attr_add(&m_ancs_c,
                                   BLE_ANCS_NOTIF_ATTR_ID_MESSAGE,
                                   m_attr_message,
                                   ATTR_DATA_SIZE);
    APP_ERROR_CHECK(err_code);
	
	/*
    err_code = ble_ancs_c_attr_add(&m_ancs_c,
                                   BLE_ANCS_NOTIF_ATTR_ID_MESSAGE_SIZE,
                                   m_attr_message_size,
                                   ATTR_DATA_SIZE);
    APP_ERROR_CHECK(err_code);*/

    /*err_code = ble_ancs_c_attr_add(&m_ancs_c,
                                   BLE_ANCS_NOTIF_ATTR_ID_DATE,
                                   m_attr_date,
                                   ATTR_DATA_SIZE);
    APP_ERROR_CHECK(err_code);

    err_code = ble_ancs_c_attr_add(&m_ancs_c,
                                   BLE_ANCS_NOTIF_ATTR_ID_POSITIVE_ACTION_LABEL,
                                   m_attr_posaction,
                                   ATTR_DATA_SIZE);
    APP_ERROR_CHECK(err_code);

    err_code = ble_ancs_c_attr_add(&m_ancs_c,
                                   BLE_ANCS_NOTIF_ATTR_ID_NEGATIVE_ACTION_LABEL,
                                   m_attr_negaction,
                                   ATTR_DATA_SIZE);
    APP_ERROR_CHECK(err_code);
	*/

    ancs_init_obj.evt_handler   = on_ancs_c_evt;
    ancs_init_obj.error_handler = apple_notification_error_handler;

    err_code = ble_ancs_c_init(&m_ancs_c, &ancs_init_obj);
    APP_ERROR_CHECK(err_code);
}

static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_ancs_c_on_db_disc_evt(&m_ancs_c, p_evt);
}

void db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}
