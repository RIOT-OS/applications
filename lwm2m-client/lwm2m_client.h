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
 * @brief       LwM2M client
 *
 * Start the app via the 'lwm2m start' command from the CLI.
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 */

#ifndef LWM2M_CLIENT_H
#define LWM2M_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generic error not described more specifically
 */
#define LWM2M_ERROR (256)

/**
 * @name    States for the client
 * @{
 */
#define LWM2M_STATE_INIT        (0)     /**< Initialized */
#define LWM2M_STATE_REG_SENT    (1)     /**< Sent initial registration */
#define LWM2M_STATE_REG_FAIL    (2)     /**< Registration failed */
#define LWM2M_STATE_REG_OK      (3)     /**< Registration succeeded */
#define LWM2M_STATE_REG_RENEW   (4)     /**< Re-registration required */
#define LWM2M_STATE_INIT_FAIL  (-1)     /**< Initialization failed */
/** @} */

/**
 * @brief  Registration interval for device management server, in seconds
 */
#define LWM2M_REG_INTERVAL   (300U)

/**
 * @brief Provides the current state of the client.
 *
 * Useful for testing.
 *
 * @return state LWM2M_STATE... macro value
 */
int lwm2m_client_state(void);

#ifdef __cplusplus
}
#endif

#endif /* LWM2M_CLIENT_H */
/** @} */
