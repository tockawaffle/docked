#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include <inttypes.h>
#include "regex.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_TAG "WIFI Driver"
#define WIFI_AUTHMODE WIFI_AUTH_WPA2_PSK
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define DEFAULT_SCAN_LIST_SIZE CONFIG_EXAMPLE_SCAN_LIST_SIZE

typedef uint8_t wifi_state_t;
#define WIFI_CONNECTED ((wifi_state_t)0)
#define WIFI_DISCONNECTED ((wifi_state_t)1)

typedef struct
{
    esp_err_t status;
    uint16_t ap_count;
    char ssids[CONFIG_EXAMPLE_SCAN_LIST_SIZE][33];
    int8_t rssis[CONFIG_EXAMPLE_SCAN_LIST_SIZE];
} wifi_scan_result_t;

typedef struct
{
    wifi_state_t state;
    char ssid[33];
    int8_t rssi;
    uint8_t last_disconnect_reason;
    esp_netif_t *netif;

    // IP information
    struct
    {
        esp_ip4_addr_t ip;
        esp_ip4_addr_t netmask;
        esp_ip4_addr_t gateway;
        bool has_ip;
        char ip_str[16];
    } ip;

    struct
    {
        esp_ip6_addr_t ip;
        bool has_ip;
        char ip_str[40];
    } ip6;
} wifi_context_t;

// Declare the global context
extern wifi_context_t g_wifi_ctx;

typedef struct
{
    esp_err_t code;
    const char *message;
    uint8_t reason_code;
    const char *reason_str;
} get_wifi_rps_t;

esp_err_t wifi_init(void);
get_wifi_rps_t wifi_connect(char *wifi_ssid, char *wifi_password);
esp_err_t wifi_disconnect(void);
esp_err_t wifi_destroy(void);
esp_err_t wifi_restart(void);
wifi_scan_result_t wifi_scan(void);

esp_err_t wifi_context_init(void);
const wifi_context_t *get_wifi_context(void);
void wifi_context_update_state(wifi_state_t new_state);
void wifi_context_update_connection(const char *ssid, int8_t rssi);
void wifi_context_update_disconnect_reason(uint8_t reason);
void wifi_context_update_ip(const esp_netif_ip_info_t *ip_info);
void wifi_context_update_ip6(const esp_ip6_addr_t *ip6_addr);