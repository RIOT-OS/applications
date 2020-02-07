/*
 * Copyright (C) 2019 Ken Bannister. All rights reserved.
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
 *
 * @}
 */

#include <stdlib.h>
#include <string.h>

#include "net/gcoap.h"
#include "phydat.h"
#include "saul_reg.h"
#include "thread.h"
#include "xtimer.h"
#include "saul_info_reporter.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static saul_reg_t *_saul_dev;
static const coap_resource_t *_resource;
static phydat_t _temp_dat;


#define POW10_MAX ( 4)
#define POW10_MIN (-4)
static int pow10[] = {1, 10, 100, 1000, 10000};

#ifndef DECIMAL_CHAR
#define DECIMAL_CHAR '.'
#endif

ssize_t saul_info_print(int16_t val, int scale, char *val_buf, int buflen) {
    assert(buflen);

    int val_len;
    if (scale >= 0 && scale <= POW10_MAX) {
        val_len = snprintf(val_buf, buflen, "%d", val * pow10[scale]);
    }
    else if (scale < 0 && scale >= POW10_MIN) {
        /* insert decimal point into string representation */
        val_len = snprintf(val_buf, buflen, "%d", val);
        if (val_len > 0) {
            char *decimal_ptr = val_buf + val_len + scale;
            memmove(decimal_ptr + 1, decimal_ptr, abs(scale) + 1);
            *decimal_ptr = DECIMAL_CHAR;
            val_len += 1;
        }
    }
    else {
        val_len = -1;
    }
    return val_len;
}

/*
 * Sends reading if observed.
 *
 * param[in] raw_value Must be int32_t to support multiplication
 */
static void _send(phydat_t temp_dat)
{
    coap_pkt_t pdu;
    uint8_t buf[40];

    if (gcoap_obs_init(&pdu, buf, 40, _resource) == GCOAP_OBS_INIT_OK) {
        coap_opt_add_format(&pdu, COAP_FORMAT_TEXT);
        size_t len = coap_opt_finish(&pdu, COAP_OPT_FINISH_PAYLOAD);

        if (pdu.payload_len >= 10) {
            len += saul_info_print(temp_dat.val[0], temp_dat.scale,
                                   (char *)pdu.payload, 10);
            gcoap_obs_send(buf, len, _resource);
        }
    }
}

int saul_info_send(void)
{
    /* take a temperature reading */
    phydat_t phy;
#ifdef BOARD_NATIVE
    memcpy(&phy, &_temp_dat, sizeof(phydat_t));
#endif

#ifdef BOARD_NATIVE
    phy.val[0] += 1;
    int res = 1;
#else
    int res = saul_reg_read(_saul_dev, &phy);
#endif

    if (res) {
        if (ENABLE_DEBUG) {
            char val_buf[10];
            if (saul_info_print(phy.val[0], phy.scale, val_buf, 10) > 0) {
                printf("temperature: %s, unit %u\n", val_buf, phy.unit);
            }
        }
    }
    else {
        DEBUG("Sensor read failure: %d\n", res);
        return -1;
    }

    _temp_dat.val[0] = phy.val[0];
    _temp_dat.scale = phy.scale;

    _send(_temp_dat);
    return 0;
}

int saul_info_init(const char *driver, const coap_resource_t *resource)
{
#ifdef BOARD_NATIVE
    (void)driver;
    _saul_dev = NULL;
    _temp_dat.val[0] = 1;
    _temp_dat.scale = 0;
    _temp_dat.unit = UNIT_TEMP_C;
#else
    _saul_dev = saul_reg_find_name(driver);
    if (_saul_dev == NULL) {
        DEBUG("Can't find SAUL reader %s\n", SAUL_INFO_DRIVER);
        return -1;
    }
#endif
    _resource = resource;
    return 0;
}

int saul_info_value(phydat_t *value)
{
    memcpy(value, &_temp_dat, sizeof(phydat_t));
    return 0;
}

