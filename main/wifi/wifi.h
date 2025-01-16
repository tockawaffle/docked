#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <inttypes.h>
#include "regex.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_TAG "WIFI Driver"
#define WIFI_AUTHMODE WIFI_AUTH_WPA2_PSK
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define DEFAULT_SCAN_LIST_SIZE CONFIG_EXAMPLE_SCAN_LIST_SIZE

typedef struct
{
    esp_err_t status;
    uint16_t ap_count;
    char ssids[CONFIG_EXAMPLE_SCAN_LIST_SIZE][33]; // 32 chars + null terminator
    int8_t rssis[CONFIG_EXAMPLE_SCAN_LIST_SIZE];
} wifi_scan_result_t;

esp_err_t wifi_init(void);
esp_err_t wifi_connect(char *wifi_ssid, char *wifi_password);
esp_err_t wifi_disconnect(void);
esp_err_t wifi_destroy(void);
esp_err_t wifi_restart(void);
wifi_scan_result_t wifi_scan(void);