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
 * @brief       Wakaama implementation for measured temperature
 *
 * @author      Ken Bannister <kb2ma@runbox.com>
 * @}
 */

#include <string.h>

#include "liblwm2m.h"
#include "measured_temp.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

/* Lookup table for static resources of measured temperature object */
#define RES_OFFSET_SENSOR_UNITS  (0)

static const char *_static_resources[] = {
    "Cel",
};

static uint8_t _read(uint16_t instance_id, int *num_dataP,
                     lwm2m_data_t **data_arrayP, lwm2m_object_t *objectP)
{
    lwm2m_measured_temp_instance_t *instance;
    DEBUG("[measured_temp::_read]");
    instance = (lwm2m_measured_temp_instance_t *)lwm2m_list_find(objectP->instanceList,
                                                                 instance_id);
    if (!instance) {
        DEBUG("[measured_temp::_read] Could not find instance\n");
        return COAP_404_NOT_FOUND;
    }

    /* if the server does not specify the amount of parameters, return all */
    if (*num_dataP == 0) {
        /* alloc memory for the single resources */
        *num_dataP = 2;
        *data_arrayP = lwm2m_data_new(*num_dataP);
        if (!(*data_arrayP)) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        (*data_arrayP)[0].id = LWM2M_MEASURED_TEMP_RES_SENSOR_VALUE;
        (*data_arrayP)[1].id = LWM2M_MEASURED_TEMP_RES_SENSOR_UNITS;
    }

    /* check which resource is wanted */
    for (int i = 0; i < *num_dataP; i++) {
        switch((*data_arrayP)[i].id) {
            case LWM2M_MEASURED_TEMP_RES_SENSOR_VALUE:
                lwm2m_data_encode_float(instance->sensor_value, *data_arrayP + i);
                break;
            case LWM2M_MEASURED_TEMP_RES_SENSOR_UNITS:
                lwm2m_data_encode_string(
                    _static_resources[RES_OFFSET_SENSOR_UNITS],
                    *data_arrayP + i);
                break;
            default:
                return COAP_404_NOT_FOUND;
        }
    }
    return COAP_205_CONTENT;
}

static uint8_t _discover(uint16_t instance_id, int *num_dataP,
                         lwm2m_data_t **data_arrayP, lwm2m_object_t *objectP)
{
    (void)instance_id;
    (void)objectP;
    DEBUG("[measured_temp::_discover]");
    if (*num_dataP == 0) {
        *num_dataP = 2;
        *data_arrayP = lwm2m_data_new(*num_dataP);
        if (!(*data_arrayP)) {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        (*data_arrayP)[0].id = LWM2M_MEASURED_TEMP_RES_SENSOR_VALUE;
        (*data_arrayP)[1].id = LWM2M_MEASURED_TEMP_RES_SENSOR_UNITS;
        return COAP_205_CONTENT;
    }
    else {
        for (int i = 0; i < *num_dataP; i++) {
            switch((*data_arrayP)[i].id) {
                case LWM2M_MEASURED_TEMP_RES_SENSOR_VALUE:
                case LWM2M_MEASURED_TEMP_RES_SENSOR_UNITS:
                    return COAP_205_CONTENT;
                default:
                    return COAP_404_NOT_FOUND;
            }
        }
        return COAP_404_NOT_FOUND;
    }
}

static uint8_t _write(uint16_t instance_id, int num_data,
                      lwm2m_data_t *data_arrayP, lwm2m_object_t *objectP)
{
    (void)instance_id;
    (void)num_data;
    (void)data_arrayP;
    (void)objectP;
    return COAP_405_METHOD_NOT_ALLOWED;
}

static uint8_t _delete(uint16_t instance_id, lwm2m_object_t *objectP)
{
    lwm2m_measured_temp_instance_t *instance;
    DEBUG("[measured_temp::_delete]");
    /* try to remote the instance from the list */
    objectP->instanceList = lwm2m_list_remove(objectP->instanceList, instance_id,
                                              (lwm2m_list_t **)&instance);
    if (!instance) {
        return COAP_404_NOT_FOUND;
    }

    /* free the allocated memory for the instance */
    lwm2m_free(instance);
    return COAP_202_DELETED;
}

static uint8_t _create(uint16_t instance_id, int num_data,
                       lwm2m_data_t *data_arrayP, lwm2m_object_t *objectP)
{
    (void)instance_id;
    (void)num_data;
    (void)data_arrayP;
    (void)objectP;
    return COAP_405_METHOD_NOT_ALLOWED;
}

static uint8_t _execute(uint16_t instance_id, uint16_t resource_id,
                        uint8_t *buffer, int length, lwm2m_object_t *objectP)
{
    (void)instance_id;
    (void)resource_id;
    (void)buffer;
    (void)length;
    (void)objectP;
    return COAP_405_METHOD_NOT_ALLOWED;
}

lwm2m_object_t *lwm2m_object_measured_temp_get(uint16_t numof)
{
    lwm2m_object_t *measured_temp_obj;
    lwm2m_measured_temp_instance_t *instance;

    measured_temp_obj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (!measured_temp_obj) {
        goto out;
    }
    memset(measured_temp_obj, 0, sizeof(lwm2m_object_t));
    measured_temp_obj->objID = LWM2M_MEASURED_TEMP_OBJECT_ID;

    for (int i = 0; i < numof; i++) {
        instance = (lwm2m_measured_temp_instance_t *)lwm2m_malloc(
                                                     sizeof(lwm2m_measured_temp_instance_t));
        if (!instance) {
            goto free_out;
        }
        memset(instance, 0, sizeof(lwm2m_measured_temp_instance_t));
        instance->shortID = i;
        instance->sensor_value = 0.0;
        measured_temp_obj->instanceList = lwm2m_list_add(
                                            measured_temp_obj->instanceList,
                                            (lwm2m_list_t *)instance);
    }

    measured_temp_obj->readFunc = _read;
    measured_temp_obj->discoverFunc = _discover;
    measured_temp_obj->writeFunc = _write;
    measured_temp_obj->deleteFunc = _delete;
    measured_temp_obj->executeFunc = _execute;
    measured_temp_obj->createFunc = _create;
    goto out;

free_out:
    lwm2m_free(measured_temp_obj);
    measured_temp_obj = NULL;
out:
    return measured_temp_obj;
}

void lwm2m_object_measured_temp_free(lwm2m_object_t *object)
{
    if (object == NULL) {
        return;
    }
    if (object->userData) {
        lwm2m_free(object->userData);
    }
    if (object->instanceList) {
        lwm2m_free(object->instanceList);
    }
    lwm2m_free(object);
}
