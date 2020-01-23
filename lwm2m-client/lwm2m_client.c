/*
 * Copyright (c) 2020 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     lwm2m_client
 *
 * @file
 * @brief       RIOT native LwM2M client
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 * @}
 */

#define ENABLE_DEBUG    (0)
#include "debug.h"


/**
 * @name    States for the client
 * @{
 */
#define LWM2M_STATE_PREINIT     (0)     /**< Not initialized yet */
/** @} */

typedef struct {
    unsigned state;
} lwm2m_client_t;

/*
static lwm2m_client_t _client = {
    .state = LWM2M_STATE_PREINIT,
};
*/
