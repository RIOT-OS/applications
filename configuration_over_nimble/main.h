#ifndef MAIN_H
#define MAIN_H

#include "ble_service.h"
#include "config.h"

union config_flashpage cfg_flashpage = {};

struct uuid_mem_mapper {
    const ble_uuid128_t* uuid;
    uint8_t* mem_p;
    size_t mem_len;
};

struct uuid_mem_mapper cfg_flashpage_access[] = {
    {
        &gatt_svr_chr_cfg_device_room_uuid,
        cfg_flashpage.config_values.device_room,
        sizeof(cfg_flashpage.config_values.device_room)
    },
    {
        &gatt_svr_chr_cfg_device_id_uuid,
        cfg_flashpage.config_values.device_id,
        sizeof(cfg_flashpage.config_values.device_id)
    },
    {
        &gatt_svr_chr_cfg_ota_host_uuid,
        cfg_flashpage.config_values.ota_host,
        sizeof(cfg_flashpage.config_values.ota_host)
    },
    {
        &gatt_svr_chr_cfg_ota_filename_uuid,
        cfg_flashpage.config_values.ota_filename,
        sizeof(cfg_flashpage.config_values.ota_filename)
    },
    {
        &gatt_svr_chr_cfg_ota_server_username_uuid,
        cfg_flashpage.config_values.ota_server_username,
        sizeof(cfg_flashpage.config_values.ota_server_username)
    },
    {
        &gatt_svr_chr_cfg_ota_server_password_uuid,
        cfg_flashpage.config_values.ota_server_password,
        sizeof(cfg_flashpage.config_values.ota_server_password)
    },
    {
        &gatt_svr_chr_cfg_wifi_ssid_uuid,
        cfg_flashpage.config_values.wifi_ssid,
        sizeof(cfg_flashpage.config_values.wifi_ssid)
    },
    {
        &gatt_svr_chr_cfg_wifi_password_uuid,
        cfg_flashpage.config_values.wifi_password,
        sizeof(cfg_flashpage.config_values.wifi_password)
    },
    {
        &gatt_svr_chr_cfg_mqtt_user_uuid,
        cfg_flashpage.config_values.mqtt_user,
        sizeof(cfg_flashpage.config_values.mqtt_user)
    },
    {
        &gatt_svr_chr_cfg_mqtt_password_uuid,
        cfg_flashpage.config_values.mqtt_password,
        sizeof(cfg_flashpage.config_values.mqtt_password)
    },
    {
        &gatt_svr_chr_cfg_mqtt_server_ip_uuid,
        cfg_flashpage.config_values.mqtt_server_ip,
        sizeof(cfg_flashpage.config_values.mqtt_server_ip)
    },
    {
        &gatt_svr_chr_cfg_mqtt_server_port_uuid,
        cfg_flashpage.config_values.mqtt_server_port,
        sizeof(cfg_flashpage.config_values.mqtt_server_port)
    },
    {
        &gatt_svr_chr_cfg_sensor_poll_interval_ms_uuid,
        cfg_flashpage.config_values.sensor_poll_interval_ms,
        sizeof(cfg_flashpage.config_values.sensor_poll_interval_ms)
    },
};

static void put_ad(uint8_t ad_type, uint8_t ad_len, const void *ad, uint8_t *buf,
                   uint8_t *len);

static void update_ad(void);

static int gap_event_cb(struct ble_gap_event *event, void *arg);

static void start_advertise(void);

/* str needs to have size of at least 48 */
void ble_uuid128_t_to_hex_str(char* str, const ble_uuid128_t *uuid);

static int gatt_svr_chr_access_device_info_manufacturer(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_device_info_model(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

static int gatt_svr_chr_access_rw_demo(
        uint16_t conn_handle, uint16_t attr_handle,
        struct ble_gatt_access_ctxt *ctxt, void *arg);

void init_default_config(void);

#endif /*MAIN_H*/
