/*
 * Copyright (c) 2020 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     app_lwm2m_client
 * @{
 *
 * @file
 * @brief       RIOT native LwM2M client
 *
 * Main entry point for the LwM2M client application.
 * 
 * Start the app via the 'lwm2m start' command from the CLI. The client will
 * register with the device management server automatically. The client then
 * will re-register with the server within LWM2M_REG_INTERVAL.
 *
 * Handles a read or observe request to /3303/0/5700 for the temerature value.
 * Notifies the observer on SAUL_INFO_INTERVAL.
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 */

#ifndef LWM2M_CLIENT_H
#define LWM2M_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    States for the client
 * @{
 */
#define LWM2M_STATE_INIT        (0)     /**< Initialized */
#define LWM2M_STATE_REG_SENT    (1)     /**< Sent initial registration */
#define LWM2M_STATE_REG_OK      (2)     /**< Registration succeeded */
#define LWM2M_STATE_REG_RENEW   (3)     /**< Re-registration required */
#define LWM2M_STATE_INFO_OK     (4)     /**< SAUL info report succeeded */
#define LWM2M_STATE_INFO_RENEW  (5)     /**< SAUL info report required */
#define LWM2M_STATE_INIT_FAIL  (-1)     /**< Initialization failed */
#define LWM2M_STATE_REG_FAIL   (-2)     /**< Registration failed */
/** @} */

/**
 * @brief Re-registration interval for device management server, in seconds
 */
#ifndef LWM2M_REG_INTERVAL
#define LWM2M_REG_INTERVAL   (300U)
#endif

/**
 * @brief Maximum length of location URI for registration interface
 */
#ifndef LWM2M_REG_LOCATION_MAXLEN
#define LWM2M_REG_LOCATION_MAXLEN (32U)
#endif

/**
 * @brief Provides the current state of the client.
 *
 * Useful for testing.
 *
 * @return LWM2M_STATE... macro value
 */
int lwm2m_client_state(void);

#ifdef __cplusplus
}
#endif

#endif /* LWM2M_CLIENT_H */
/** @} */
