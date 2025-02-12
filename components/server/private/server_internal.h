#ifndef SERVER_INTERNAL_H
#define SERVER_INTERNAL_H

#include "esp_log.h"
#include "http_types.h"

#define UPDATE_CHECK_URL "https://your-domain.com/version.json"
#define OTA_SERVER_PORT 80

// Embedded public key for verification (generated offline)
extern const uint8_t public_key[];
extern const size_t public_key_len;

extern ota_status_t g_ota_status;

static esp_err_t verify_firmware_signature(const uint8_t *firmware, size_t size, const uint8_t *signature);

#endif