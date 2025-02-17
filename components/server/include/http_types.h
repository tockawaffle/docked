#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

#include <esp_http_server.h>
#include <esp_ota_ops.h>
#include <esp_http_client.h>
#include <cJSON.h>
#include <mbedtls/base64.h>
#include <mbedtls/rsa.h>
#include <mbedtls/sha256.h>

typedef struct
{
    const char *current_version;
    const char *latest_version;
    bool update_available;
    unsigned char signature[512];
} ota_status_t;

esp_err_t init_ota_server(void);
esp_err_t check_for_update(void);

#endif