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
 * @brief       SAUL based LwM2M information reporter
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 */

#ifndef SAUL_INFO_REPORTER_H
#define SAUL_INFO_REPORTER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Interval between readings, in seconds
 */
#ifndef SAUL_INFO_INTERVAL
#define SAUL_INFO_INTERVAL    (60)
#endif

/**
 * @brief Initialize on the information/resource to report
 *
 * @param[in] driver   name of SAUL driver to retrieve value
 * @param[in] resource CoAP resource to report on
 *
 * @return 0 if success
 * @return <0 on failure
 */
int saul_info_init(const char *driver, const coap_resource_t *resource);

/**
 * @brief Prints a sensor value to a char buffer.
 *
 * Assumes val and scale parameters are from a phydat_t.
 *
 * @param[in] val       sensor value
 * @param[in] scale     scale for sensor value
 * @param[out] val_buf  target for char output
 * @param[in] buflen    length of output buffer
 *
 * @return Length used in buffer
 * @return <0 if unable to print, likely due to insufficient buffer space
 */
ssize_t saul_info_print(int16_t val, int scale, char *val_buf, int buflen);
 
/**
 * @brief Send an information report to the server if observing
 *
 * Also takes a reading as needed.
 *
 * @return 0 if success
 * @return <0 on failure
 */
int saul_info_send(void);

/**
 * @brief Provides the current temperature value.
 *
 * @param[out] value
 *
 * @return 0 if success
 * @return <0 on failure
 */
int saul_info_value(phydat_t *value);

#ifdef __cplusplus
}
#endif

#endif /* SAUL_INFO_REPORTER_H */
/** @} */
