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
 * @brief Registers with server.
 *
 * @return 0 on success
 * @return -EALREADY if already started
 * @return -ECOMM if message send failed
 * @return -LWM2M_ERROR if some other error occurred
 */
int lwm2m_client_start(void);

#ifdef __cplusplus
}
#endif

#endif /* LWM2M_CLIENT_H */
/** @} */
