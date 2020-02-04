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
 * @brief       LwM2M client CLI
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 */

#ifndef LWM2M_CLI_H
#define LWM2M_CLI_H

#include "kernel_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Starts the CLI thread.
 *
 * Runs at a lower priority than the main LwM2M thread.
 *
 * @param lwm2m_pid PID for the main LwM2M thread
 *
 * @return 0 if thread started OK
 * @return -EALREADY if CLI thread already started
 */
int lwm2m_cli_start(kernel_pid_t lwm2m_pid);

#ifdef __cplusplus
}
#endif

#endif /* LWM2M_CLI_H */
/** @} */
