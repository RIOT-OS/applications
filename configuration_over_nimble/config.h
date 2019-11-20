#ifndef CONFIG_H
#define CONFIG_H

#include <assert.h>

#define ALIGNMENT_ATTR
#define FLASHPAGE_NUM_CONFIG FLASHPAGE_NUMOF - 1

#define CONFIG_DEVICE_ROOM "CONFIG_DEVICE_ROOM"
#define CONFIG_DEVICE_ID "CONFIG_DEVICE_ID"
#define CONFIG_OTA_HOST "CONFIG_OTA_HOST"
#define CONFIG_OTA_FILENAME "CONFIG_OTA_FILENAME"
#define CONFIG_OTA_SERVER_USERNAME "CONFIG_OTA_SERVER_USERNAME"
#define CONFIG_OTA_SERVER_PASSWORD "CONFIG_OTA_SERVER_PASSWORD"
#define CONFIG_ESP_WIFI_SSID "CONFIG_ESP_WIFI_SSID"
#define CONFIG_ESP_WIFI_PASSWORD "CONFIG_ESP_WIFI_PASSWORD"
#define CONFIG_MQTT_USER "CONFIG_MQTT_USER"
#define CONFIG_MQTT_PASSWORD "CONFIG_MQTT_PASSWORD"
#define CONFIG_MQTT_SERVER_IP "CONFIG_MQTT_SERVER_IP"
#define CONFIG_MQTT_SERVER_PORT "CONFIG_MQTT_SERVER_PORT"
#define CONFIG_SENSOR_POLL_INTERVAL_MS "CONFIG_SENSOR_POLL_INTERVAL_MS"

struct config_values {
    uint8_t device_room[256];
    uint8_t device_id[256];
    uint8_t ota_host[256];
    uint8_t ota_filename[256];
    uint8_t ota_server_username[256];
    uint8_t ota_server_password[256];
    uint8_t wifi_ssid[256];
    uint8_t wifi_password[256];
    uint8_t mqtt_user[256];
    uint8_t mqtt_password[256];
    uint8_t mqtt_server_ip[256];
    uint8_t mqtt_server_port[32];
    uint8_t sensor_poll_interval_ms[32];
};

static_assert(FLASHPAGE_SIZE >= 4096, "condition not met: FLASHPAGE_SIZE >= 4096");
union config_flashpage {
    struct config_values config_values;
    uint8_t page_mem[FLASHPAGE_SIZE] ALIGNMENT_ATTR;
};

void config_read(union config_flashpage* config);
void config_store(union config_flashpage* config);

#endif /*CONFIG_H*/
