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
 * @brief       Wakaama temperature measurement object
 *
 * Supports only sensor value and units.
 * 
 * @see http://www.openmobilealliance.org/tech/profiles/lwm2m/3303.xml
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 */

#ifndef MEASURED_TEMP_H
#define MEASURED_TEMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "liblwm2m.h"

/**
 * @brief LWM2M ID for the measured temperature object
 */
#define LWM2M_MEASURED_TEMP_OBJECT_ID   (3303)

/**
 * @brief Resource IDs for the measured temperature object
 */
typedef enum {
    LWM2M_MEASURED_TEMP_RES_SENSOR_VALUE = 5700,
    LWM2M_MEASURED_TEMP_RES_SENSOR_UNITS,
    LWM2M_MEASURED_TEMP_RES_MEASURED_MIN = 5601,
    LWM2M_MEASURED_TEMP_RES_MEASURED_MAX,
    LWM2M_MEASURED_TEMP_RES_RANGE_MIN,
    LWM2M_MEASURED_TEMP_RES_RANGE_MAX,
    LWM2M_MEASURED_TEMP_RES_RESET_MEASURED
} lwm2m_measured_temp_resource_t;

/**
 * @brief Measured temperature object instance descriptor
 */
typedef struct l2mwm_measured_temp_instance {
    struct measured_temp_instance *next;  /**< matches lwm2m_list_t::next */
    uint16_t shortID;                     /**< matches lwm2m_list_t::id */
    double sensor_value;
} lwm2m_measured_temp_instance_t;

/**
 * @brief Creates a measured temperature object and @p numof instances
 *
 * @param[in] numof number of instances to create
 * 
 * @return Pointer to the created object
 * @return NULL if could not create the object
 */
lwm2m_object_t *lwm2m_object_measured_temp_get(uint16_t numof);

/**
 * @brief Frees allocated memory for a measured temperature object
 *
 * @param[in] object pointer to the object
 */
void lwm2m_object_measured_temp_free(lwm2m_object_t *object);

#ifdef __cplusplus
}
#endif

#endif /* MEASURED_TEMP_H */
/** @} */
