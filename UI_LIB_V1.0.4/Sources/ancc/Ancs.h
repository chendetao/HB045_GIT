#ifndef __ANCS_H___
#define __ANCS_H___

#include "ble_ancs_c.h"

extern ble_ancs_c_t              m_ancs_c;
extern dm_handle_t               m_peer_handle;
extern ble_db_discovery_t        m_ble_db_discovery;                       /**< Structure used to identify the DB Discovery module. */
extern void ancs_service_init(void);
extern int slave_security_request(void);
extern void db_discovery_init(void);
extern bool ancs_perform_notification_action( bool action );

#endif
