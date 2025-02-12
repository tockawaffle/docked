#pragma once

#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SSID_LENGTH 33      // 32 chars + null terminator
#define MAX_PASSWORD_LENGTH 65   // 64 chars + null terminator

typedef struct {
    char ssid[MAX_SSID_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} wifi_credentials_t;

/**
 * @brief Save WiFi credentials to NVS
 * Stores both the SSID and password as binary blobs to preserve encoding
 * 
 * @param ssid Network SSID
 * @param password Network password
 * @return esp_err_t 
 */
esp_err_t save_wifi_config(const char *ssid, const char *password);

/**
 * @brief Read WiFi credentials from NVS
 * 
 * @param credentials Pointer to store the credentials
 * @return esp_err_t 
 */
esp_err_t read_wifi_config(wifi_credentials_t *credentials);

/**
 * @brief Remove stored WiFi credentials
 * 
 * @return esp_err_t 
 */
esp_err_t remove_wifi_config(void);

#ifdef __cplusplus
}
#endif