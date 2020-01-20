/*
 * Copyright (C) 2019 Ken Bannister. All rights reserved.
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
 *
 * @}
 */

#include <stdlib.h>
#include <string.h>
#include "liblwm2m.h"
#include "phydat.h"
#include "saul_reg.h"
#include "thread.h"
#include "xtimer.h"

#include "measured_temp.h"
#include "temp_reader.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static char _msg_stack[TEMP_READER_STACK_SIZE];
static kernel_pid_t _pid = KERNEL_PID_UNDEF;

static saul_reg_t* _saul_dev;
static lwm2m_context_t *_lwm2m_ctx;
static lwm2m_uri_t _uri;
static lwm2m_measured_temp_instance_t *_temp_instance;

#define POW10_MAX ( 4)
#define POW10_MIN (-4)
static int pow10[] = {1, 10, 100, 1000, 10000};
#ifndef DECIMAL_CHAR
#define DECIMAL_CHAR '.'
#endif

/*
 * Takes a sensor reading at a defined interval; does not exit.
 *
 * For native, starts at an arbitrary value and increments it with each reading.
 */
static void *_read_loop(void *arg)
{
    (void)arg;
    /* take a temperature reading */
    phydat_t phy;
#ifdef BOARD_NATIVE
    phy.val[0] = 10;
#endif

    while (1) {
#ifdef BOARD_NATIVE
        phy.val[0] += 1;
        phy.scale = 0;
        phy.unit = UNIT_TEMP_C;
        int res = 1;
#else
        int res = saul_reg_read(_saul_dev, &phy);
#endif
        if (res) {
            if (phy.scale >= 0 && phy.scale <= POW10_MAX) {
                _temp_instance->sensor_value = (double)phy.val[0] * pow10[phy.scale];
                DEBUG("temperature: %d, unit %u\n", phy.val[0] * pow10[phy.scale],
                      phy.unit);
            }
            else if (phy.scale < 0 && phy.scale >= POW10_MIN) {
                _temp_instance->sensor_value = (double)phy.val[0] / pow10[abs(phy.scale)];
                if (ENABLE_DEBUG) {
                    /* insert decimal point into string representation */
                    char val_buf[10];
                    int val_len = sprintf(val_buf, "%d", phy.val[0]);
                    char *decimal_ptr = val_buf + val_len + phy.scale;
                    memmove(decimal_ptr + 1, decimal_ptr, abs(phy.scale) + 1);
                    *decimal_ptr = DECIMAL_CHAR;
                    printf("temperature: %s, unit %u\n", val_buf, phy.unit);
                }
            }
            else {
                DEBUG("scale out of range: %d\n", phy.scale);
                goto sleep;
            }

            /* mark changed for observers */
            lwm2m_resource_value_changed(_lwm2m_ctx, &_uri);
        }
        else {
            DEBUG("Sensor read failure: %d\n", res);
        }

        sleep:
        xtimer_sleep(TEMP_READER_INTERVAL);
    }
    return 0;
}


int temp_reader_start(const char *reader_name, lwm2m_context_t *lwm2m_ctx,
                     lwm2m_measured_temp_instance_t *instance)
{
    if (_pid != KERNEL_PID_UNDEF) {
        return 0;
    }
#ifdef BOARD_NATIVE
    (void)reader_name;
    _saul_dev = NULL;
    DEBUG("Ignoring sensor init on native");
#else
    _saul_dev = saul_reg_find_name(reader_name);
    if (_saul_dev == NULL) {
        DEBUG("Can't find SAUL reader %s\n", reader_name);
        return -1;
    }
#endif

    _lwm2m_ctx = lwm2m_ctx;
    _temp_instance = instance;

    _uri.flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID |
                LWM2M_URI_FLAG_RESOURCE_ID;
    _uri.objectId = LWM2M_MEASURED_TEMP_OBJECT_ID;
    _uri.instanceId = instance->shortID;
    _uri.resourceId = LWM2M_MEASURED_TEMP_RES_SENSOR_VALUE;

    /* Priority is main-6 because LwM2M is main-1, and network stack is below
     * that. */
    _pid = thread_create(_msg_stack, sizeof(_msg_stack),
                         THREAD_PRIORITY_MAIN - 6, THREAD_CREATE_STACKTEST,
                         _read_loop, NULL, "temp reader");
    return 0;
}

