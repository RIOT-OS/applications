/*
 * Copyright (C) 2019 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     applications
 * @{
 *
 * @file
 * @brief       BLE service for configuration of a device using NimBLE and flashpage
 *
 * Test this application e.g. with Nordics "nRF Connect"-App
 * iOS: https://itunes.apple.com/us/app/nrf-connect/id1054362403
 * Android: https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp
 *
 * Hint: There is an app specifically written for this service.
 * You can find it here:
 * https://github.com/HendrikVE/smarthome2/tree/master/AndroidApp/WindowAlarmConfig
 *
 * @author      Hendrik van Essen <hendrik.ve@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "periph/pm.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_gatt.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "main.h"

#define STR_ANSWER_BUFFER_SIZE 100

static char str_answer[STR_ANSWER_BUFFER_SIZE];

static const char device_name[] = "Lord NimBLEr";
static uint8_t own_addr_type;

static void put_ad(uint8_t ad_type, uint8_t ad_len, const void *ad, uint8_t *buf,
                   uint8_t *len)
{
    buf[(*len)++] = ad_len + 1;
    buf[(*len)++] = ad_type;

    memcpy(&buf[*len], ad, ad_len);

    *len += ad_len;
}

static void update_ad(void)
{
    uint8_t ad[BLE_HS_ADV_MAX_SZ];
    uint8_t ad_len = 0;
    uint8_t ad_flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

    put_ad(BLE_HS_ADV_TYPE_FLAGS, 1, &ad_flags, ad, &ad_len);
    put_ad(BLE_HS_ADV_TYPE_COMP_NAME, sizeof(device_name), device_name, ad, &ad_len);

    ble_gap_adv_set_data(ad, ad_len);
}

static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status) {
                start_advertise();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            start_advertise();
            break;
    }

    return 0;
}

static void start_advertise(void)
{
    struct ble_gap_adv_params advp;
    int rc;

    memset(&advp, 0, sizeof advp);
    advp.conn_mode = BLE_GAP_CONN_MODE_UND;
    advp.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &advp, gap_event_cb, NULL);

    assert(rc == 0);
    (void)rc;
}

/* str needs to have size of at least 48 */
void ble_uuid128_t_to_hex_str(char* str, const ble_uuid128_t *uuid)
{

    str[0]= '\0';
    char hex[3];

    for (int i = 15; i >= 0; i--) {
        snprintf(hex, 3, "%02x", uuid->value[i]);
        strcat(str, hex);

        if (i != 0) {
            strcat(str, "-");
        }
    }
}

static int gatt_svr_chr_access_device_info_manufacturer(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    puts("service 'device info: manufacturer' callback triggered");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    snprintf(str_answer, STR_ANSWER_BUFFER_SIZE,
             "This is RIOT! (Version: %s)\n", RIOT_VERSION);
    puts(str_answer);

    int rc = os_mbuf_append(ctxt->om, str_answer, strlen(str_answer));

    puts("");

    return rc;
}

static int gatt_svr_chr_access_device_info_model(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    puts("service 'device info: model' callback triggered");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    snprintf(str_answer, STR_ANSWER_BUFFER_SIZE,
             "You are running RIOT on a(n) %s board, "
             "which features a(n) %s MCU.", RIOT_BOARD, RIOT_MCU);
    puts(str_answer);

    int rc = os_mbuf_append(ctxt->om, str_answer, strlen(str_answer));

    puts("");

    return rc;
}

static int gatt_svr_chr_access_rw_demo(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    puts("service 'rw demo' callback triggered");

    (void) conn_handle;
    (void) attr_handle;
    (void) arg;

    int rc = 0;

    const ble_uuid_t* accessed_uuid = ctxt->chr->uuid;

    int cmp = ble_uuid_cmp(accessed_uuid,
            (ble_uuid_t*) &gatt_svr_chr_cfg_restart_uuid.u);
    if (cmp == 0) {
        if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
            puts("store config");
            config_store(&cfg_flashpage);

            puts("restart");
            pm_reboot();
        }

        return 0;
    }

    int config_item_count = sizeof(cfg_flashpage_access) / sizeof(cfg_flashpage_access[0]);

    char uuid_str[48];
    for (int i = 0; i < config_item_count; i++) {

        struct uuid_mem_mapper *mapper = &cfg_flashpage_access[i];

        if (ble_uuid_cmp((ble_uuid_t*) mapper->uuid, accessed_uuid) == 0) {

            ble_uuid128_t_to_hex_str(uuid_str, mapper->uuid);

            switch (ctxt->op) {

                case BLE_GATT_ACCESS_OP_READ_CHR:

                    /* be safe in case flash still contains garbage data */
                    mapper->mem_p[mapper->mem_len - 1] = '\0';

                    printf("current value for uuid '%s': '%s'\n",
                           uuid_str,
                           (char*) mapper->mem_p);

                    /* send given data to the client */
                    rc = os_mbuf_append(ctxt->om,
                            mapper->mem_p, strlen((char*) mapper->mem_p));

                    break;

                case BLE_GATT_ACCESS_OP_WRITE_CHR:

                    printf("old value for uuid '%s': '%s'\n",
                           uuid_str,
                           (char*) mapper->mem_p);

                    uint16_t om_len;
                    om_len = OS_MBUF_PKTLEN(ctxt->om);

                    /* read sent data */
                    rc = ble_hs_mbuf_to_flat(ctxt->om, mapper->mem_p,
                                             mapper->mem_len, &om_len);
                    /* we need to null-terminate the received string */
                    mapper->mem_p[om_len] = '\0';

                    printf("new value for uuid '%s': '%s'\n",
                           uuid_str,
                           (char*) mapper->mem_p);

                    break;

                default:
                    puts("unhandled operation!");
                    rc = 1;
                    break;
            }

            return rc;
        }
    }

    puts("unhandled uuid!");
    return 1;
}

void init_default_config(void)
{

    memcpy(cfg_flashpage.config_values.device_room,
           CONFIG_DEVICE_ROOM,
           sizeof(cfg_flashpage.config_values.device_room)
    );

    memcpy(cfg_flashpage.config_values.device_id,
           CONFIG_DEVICE_ID,
           sizeof(cfg_flashpage.config_values.device_id)
    );

    memcpy(cfg_flashpage.config_values.ota_host,
           CONFIG_OTA_HOST,
           sizeof(cfg_flashpage.config_values.ota_host)
    );

    memcpy(cfg_flashpage.config_values.ota_filename,
           CONFIG_OTA_FILENAME,
           sizeof(cfg_flashpage.config_values.ota_filename)
    );

    memcpy(cfg_flashpage.config_values.ota_server_username,
           CONFIG_OTA_SERVER_USERNAME,
           sizeof(cfg_flashpage.config_values.ota_server_username)
    );

    memcpy(cfg_flashpage.config_values.ota_server_password,
           CONFIG_OTA_SERVER_PASSWORD,
           sizeof(cfg_flashpage.config_values.ota_server_password)
    );

    memcpy(cfg_flashpage.config_values.wifi_ssid,
           CONFIG_ESP_WIFI_SSID,
           sizeof(cfg_flashpage.config_values.wifi_ssid)
    );

    memcpy(cfg_flashpage.config_values.wifi_password,
           CONFIG_ESP_WIFI_PASSWORD,
           sizeof(cfg_flashpage.config_values.wifi_password)
    );

    memcpy(cfg_flashpage.config_values.mqtt_user,
           CONFIG_MQTT_USER,
           sizeof(cfg_flashpage.config_values.mqtt_user)
    );

    memcpy(cfg_flashpage.config_values.mqtt_password,
           CONFIG_MQTT_PASSWORD,
           sizeof(cfg_flashpage.config_values.mqtt_password)
    );

    memcpy(cfg_flashpage.config_values.mqtt_server_ip,
           CONFIG_MQTT_SERVER_IP,
           sizeof(cfg_flashpage.config_values.mqtt_server_ip)
    );

    memcpy(cfg_flashpage.config_values.mqtt_server_port,
           CONFIG_MQTT_SERVER_PORT,
           sizeof(cfg_flashpage.config_values.mqtt_server_port)
    );

    memcpy(cfg_flashpage.config_values.sensor_poll_interval_ms,
           CONFIG_SENSOR_POLL_INTERVAL_MS,
           sizeof(cfg_flashpage.config_values.sensor_poll_interval_ms)
    );

    memcpy(cfg_flashpage.config_values.sensor_poll_interval_ms,
           CONFIG_SENSOR_POLL_INTERVAL_MS,
           sizeof(cfg_flashpage.config_values.sensor_poll_interval_ms)
    );
}

int main(void)
{
    puts("Configurator Application");

    config_read(&cfg_flashpage);

    /*
     * if device_id is an empty string, the config flash page was probably
     * not initialized yet, because it is the first run of the application
     */
    uint8_t first_byte = cfg_flashpage.config_values.device_id[0];

    /* check if first ascii is within range of allowed characters */
    if (first_byte >= 32 && first_byte <= 126) {
        puts("use stored config from flash");
    }
    else {
        puts("use defaults as the flash seems to be empty "
             "(first start -> config was never stored)");

        init_default_config();
    }

    int rc = 0;

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    assert(rc == 0);

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    assert(rc == 0);

    /* set the device name */
    ble_svc_gap_device_name_set(device_name);

    /* initialize the GAP and GATT services */
    ble_svc_gap_init();
    ble_svc_gatt_init();
    /* XXX: seems to be needed to apply the added services */
    ble_gatts_start();

    /* make sure synchronization of host and controller is done, this should
     * always be the case */
    while (!ble_hs_synced()) {}

    /* configure device address */
    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    assert(rc == 0);
    (void)rc;

    /* generate the advertising data */
    update_ad();

    /* start to advertise this node */
    puts("start advertising");
    start_advertise();

    return 0;
}
