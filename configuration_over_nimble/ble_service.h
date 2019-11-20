#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include "host/ble_uuid.h"

#define GATT_DEVICE_INFO_UUID                   0x180A
#define GATT_MANUFACTURER_NAME_UUID             0x2A29
#define GATT_MODEL_NUMBER_UUID                  0x2A24

/* CONFIG SERVICE UUID */
/* UUID = 2fa1dab8-3eef-40fc-8540-7fc496a10d75 */
static const ble_uuid128_t gatt_svr_svc_cfg_uuid
        = BLE_UUID128_INIT(0x75, 0x0d, 0xa1, 0x96, 0xc4, 0x7f, 0x40, 0x85, 0xfc,
                           0x40, 0xef, 0x3e, 0xb8, 0xda, 0xa1, 0x2f);

/* CONFIG DEVICE */
/* UUID = d3491796-683b-4b9c-aafb-f51a35459d43 */
static const ble_uuid128_t gatt_svr_chr_cfg_device_room_uuid
        = BLE_UUID128_INIT(0x43, 0x9d, 0x45, 0x35, 0x1a, 0xf5, 0xfb, 0xaa, 0x9c,
                           0x4b, 0x3b, 0x68, 0x96, 0x17, 0x49, 0xd3);

/* UUID = 4745e11f-b403-4cfb-83bb-710d46897875 */
static const ble_uuid128_t gatt_svr_chr_cfg_device_id_uuid
        = BLE_UUID128_INIT(0x75, 0x78, 0x89, 0x46, 0x0d, 0x71, 0xbb, 0x83, 0xfb,
                           0x4c, 0x03, 0xb4, 0x1f, 0xe1, 0x45, 0x47);

/* CONFIG OTA */
/* UUID = 2f44b103-444c-48f5-bf60-91b81dfa0a25 */
static const ble_uuid128_t gatt_svr_chr_cfg_ota_host_uuid
        = BLE_UUID128_INIT(0x25, 0x0a, 0xfa, 0x1d, 0xb8, 0x91, 0x60, 0xbf, 0xf5,
                           0x48, 0x4c, 0x44, 0x03, 0xb1, 0x44, 0x2f);

/* UUID = 4b95d245-db08-4c56-98f9-738faa8cfbb6 */
static const ble_uuid128_t gatt_svr_chr_cfg_ota_filename_uuid
        = BLE_UUID128_INIT(0xb6, 0xfb, 0x8c, 0xaa, 0x8f, 0x73, 0xf9, 0x98, 0x56,
                           0x4c, 0x08, 0xdb, 0x45, 0xd2, 0x95, 0x4b);

/* UUID = 1c93dce2-3796-4027-9f55-6d251c41dd34 */
static const ble_uuid128_t gatt_svr_chr_cfg_ota_server_username_uuid
        = BLE_UUID128_INIT(0x34, 0xdd, 0x41, 0x1c, 0x25, 0x6d, 0x55, 0x9f, 0x27,
                           0x40, 0x96, 0x37, 0xe2, 0xdc, 0x93, 0x1c);

/* UUID = 0e837309-5336-45a3-9b69-d0f7134f30ff */
static const ble_uuid128_t gatt_svr_chr_cfg_ota_server_password_uuid
        = BLE_UUID128_INIT(0xff, 0x30, 0x4f, 0x13, 0xf7, 0xd0, 0x69, 0x9b, 0xa3,
                           0x45, 0x36, 0x53, 0x09, 0x73, 0x83, 0x0e);

/* CONFIG WiFi */
/* UUID = 8ca0bf1d-bb5d-4a66-9191-341fd805e288 */
static const ble_uuid128_t gatt_svr_chr_cfg_wifi_ssid_uuid
        = BLE_UUID128_INIT(0x88, 0xe2, 0x05, 0xd8, 0x1f, 0x34, 0x91, 0x91, 0x66,
                           0x4a, 0x5d, 0xbb, 0x1d, 0xbf, 0xa0, 0x8c);

/* UUID = fa41c195-ae99-422e-8f1f-0730702b3fc5 */
static const ble_uuid128_t gatt_svr_chr_cfg_wifi_password_uuid
        = BLE_UUID128_INIT(0xc5, 0x3f, 0x2b, 0x70, 0x30, 0x07, 0x1f, 0x8f, 0x2e,
                           0x42, 0x99, 0xae, 0x95, 0xc1, 0x41, 0xfa);

/* CONFIG MQTT */
/* UUID = 69150609-18f8-4523-a41f-6d9a01d2e08d */
static const ble_uuid128_t gatt_svr_chr_cfg_mqtt_user_uuid
        = BLE_UUID128_INIT(0x8d, 0xe0, 0xd2, 0x01, 0x9a, 0x6d, 0x1f, 0xa4, 0x23,
                           0x45, 0xf8, 0x18, 0x09, 0x06, 0x15, 0x69);

/* UUID = 8bebec77-ea21-4c14-9d64-dbec1fd5267c */
static const ble_uuid128_t gatt_svr_chr_cfg_mqtt_password_uuid
        = BLE_UUID128_INIT(0x7c, 0x26, 0xd5, 0x1f, 0xec, 0xdb, 0x64, 0x9d, 0x14,
                           0x4c, 0x21, 0xea, 0x77, 0xec, 0xeb, 0x8b);

/* UUID = e3b150fb-90a2-4cd3-80c5-b1189e110ef1 */
static const ble_uuid128_t gatt_svr_chr_cfg_mqtt_server_ip_uuid
        = BLE_UUID128_INIT(0xf1, 0x0e, 0x11, 0x9e, 0x18, 0xb1, 0xc5, 0x80, 0xd3,
                           0x4c, 0xa2, 0x90, 0xfb, 0x50, 0xb1, 0xe3);

/* UUID = 4eeff953-0f5e-43ee-b1be-1783a8190b0d */
static const ble_uuid128_t gatt_svr_chr_cfg_mqtt_server_port_uuid
        = BLE_UUID128_INIT(0x0d, 0x0b, 0x19, 0xa8, 0x83, 0x17, 0xbe, 0xb1, 0xee,
                           0x43, 0x5e, 0x0f, 0x53, 0xf9, 0xef, 0x4e);

/* CONFIG SENSOR */
/* UUID = 68011c92-854a-4f2c-a94c-5ee37dc607c3 */
static const ble_uuid128_t gatt_svr_chr_cfg_sensor_poll_interval_ms_uuid
        = BLE_UUID128_INIT(0xc3, 0x07, 0xc6, 0x7d, 0xe3, 0x5e, 0x4c, 0xa9, 0x2c,
                           0x4f, 0x4a, 0x85, 0x92, 0x1c, 0x01, 0x68);

/* RESTART */
/* UUID = 890f7b6f-cecc-4e3e-ade2-5f2907867f4b */
static const ble_uuid128_t gatt_svr_chr_cfg_restart_uuid
        = BLE_UUID128_INIT(0x4b, 0x7f, 0x86, 0x07, 0x29, 0x5f, 0xe2, 0xad, 0x3e,
                           0x4e, 0xcc, 0xce, 0x6f, 0x7b, 0x0f, 0x89);

static int gatt_svr_chr_access_device_info_manufacturer(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_device_info_model(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_rw_demo(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

/* define several bluetooth services for our device */
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    /*
     * access_cb defines a callback for read and write access events on
     * given characteristics
     */
    {
        /* Service: Device Information */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(GATT_DEVICE_INFO_UUID),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /* Characteristic: * Manufacturer name */
            .uuid = BLE_UUID16_DECLARE(GATT_MANUFACTURER_NAME_UUID),
            .access_cb = gatt_svr_chr_access_device_info_manufacturer,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            /* Characteristic: Model number string */
            .uuid = BLE_UUID16_DECLARE(GATT_MODEL_NUMBER_UUID),
            .access_cb = gatt_svr_chr_access_device_info_model,
            .flags = BLE_GATT_CHR_F_READ,
        }, {
            0, /* No more characteristics in this service */
        }, }
    },
    {
        /* Service: Config */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = (ble_uuid_t*) &gatt_svr_svc_cfg_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /* Characteristic: Config device room */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_device_room_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config device id */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_device_id_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config ota host */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_ota_host_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config ota filename */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_ota_filename_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config ota server username */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_ota_server_username_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config ota server password */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_ota_server_password_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config wifi ssid */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_wifi_ssid_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config wifi password */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_wifi_password_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config mqtt user */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_mqtt_user_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config mqtt password */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_mqtt_password_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config mqtt server ip */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_mqtt_server_ip_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config mqtt server port */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_mqtt_server_port_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config sensor poll interval ms */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_sensor_poll_interval_ms_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            /* Characteristic: Config save and restart */
            .uuid = (ble_uuid_t*) &gatt_svr_chr_cfg_restart_uuid.u,
            .access_cb = gatt_svr_chr_access_rw_demo,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        },{
            0, /* No more characteristics in this service */
        }, }
    },
    {
        0, /* No more services */
    },
};

#endif /*BLE_SERVICE_H*/
