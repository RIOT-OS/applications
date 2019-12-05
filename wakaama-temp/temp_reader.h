/*
 * Copyright (c) 2019 Ken Bannister. All rights reserved.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     app_wakaama_temp
 * @{
 *
 * @file
 * @brief       SAUL reader for LwM2M temperature
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 */

#ifndef TEMP_READER_H
#define TEMP_READER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liblwm2m.h"
#include "measured_temp.h"

/**
 * @brief Stack size for module thread
 */
#ifndef TEMP_READER_STACK_SIZE
#define TEMP_READER_STACK_SIZE  (THREAD_STACKSIZE_LARGE)
#endif

/**
 * @brief Interval between readings, in seconds
 */
#ifndef TEMP_READER_INTERVAL
#define TEMP_READER_INTERVAL    (60)
#endif

/**
 * @brief Start thread to measure temperature on the reader interval
 *
 * @param[in] reader_name   Name for the SAUL sensor
 * @param[in] lwm2m_ctx     LwM2M context so can notify of temperature
 * @param[in,out] instance  LwM2M measured temperature object instance to hold
 *                          data
 *
 * @return 0 if start OK
 * @return -1 if start failed
 */
int temp_reader_start(const char *reader_name, lwm2m_context_t *lwm2m_ctx,
                      lwm2m_measured_temp_instance_t *instance);

#ifdef __cplusplus
}
#endif

#endif /* TEMP_READER_H */
/** @} */
