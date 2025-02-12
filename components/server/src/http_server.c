#include "server_internal.h"
#include "wifi_c_types.h"

static httpd_handle_t server = NULL;
ota_status_t g_ota_status = {
    .current_version = "1.0.0",
    .latest_version = NULL,
    .update_available = false};

static esp_err_t verify_firmware_signature(const uint8_t *firmware, size_t size, const uint8_t *signature)
{
    mbedtls_rsa_context rsa;
    unsigned char hash[32];

    mbedtls_rsa_init(&rsa);
    mbedtls_sha256(firmware, size, hash, 0);

    if (mbedtls_rsa_import_raw(&rsa,
                               public_key, public_key_len,
                               NULL, 0,
                               NULL, 0,
                               NULL, 0,
                               NULL, 0) != 0)
    {
        mbedtls_rsa_free(&rsa);
        return ESP_FAIL;
    }

    int ret = mbedtls_rsa_pkcs1_verify(&rsa,
                                       MBEDTLS_MD_SHA256,
                                       32,
                                       hash,
                                       signature);

    mbedtls_rsa_free(&rsa);
    return ret == 0 ? ESP_OK : ESP_FAIL;
}

static esp_err_t handle_options(httpd_req_t *req)
{
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    httpd_resp_set_status(req, "204 No Content");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

void format_uptime(int64_t uptime_us, char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size < 16)
    { // Minimum size for "XXXd XXh XXm XXs"
        return;
    }

    // Convert to seconds first (integer division by 1,000,000)
    int64_t total_seconds = uptime_us / 1000000;

    // Calculate each time unit
    int days = (int)(total_seconds / 86400);
    int hours = (int)((total_seconds % 86400) / 3600);
    int minutes = (int)((total_seconds % 3600) / 60);
    int seconds = (int)(total_seconds % 60);

    // Format with bounds checking
    int written = snprintf(buffer, buffer_size, "%dd %02dh %02dm %02ds",
                           days, hours, minutes, seconds);

    // Ensure null termination in case of truncation
    if (written >= buffer_size)
    {
        buffer[buffer_size - 1] = '\0';
    }
}

static esp_err_t handle_get_status(httpd_req_t *req)
{
    // ESP_LOGI("HTTP", "Handling status request");

    // Set all CORS headers
    httpd_resp_set_status(req, "200 OK");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Accept");
    httpd_resp_set_hdr(req, "Access-Control-Max-Age", "3600");
    httpd_resp_set_type(req, "application/json");

    cJSON *root = cJSON_CreateObject();
    if (!root)
    {
        ESP_LOGE("HTTP", "Failed to create JSON object");
        return ESP_FAIL;
    }

    // Get the wifi context
    const wifi_context_t *wifi_ctx = get_wifi_context();

    char uptime_str[32]; // Buffer size of 32 is plenty for the format
    int64_t uptime_us = esp_timer_get_time();
    format_uptime(uptime_us, uptime_str, sizeof(uptime_str));

    cJSON_AddStringToObject(root, "current_version", g_ota_status.current_version);
    cJSON_AddStringToObject(root, "latest_version", g_ota_status.latest_version ? g_ota_status.latest_version : "unknown");
    cJSON_AddBoolToObject(root, "update_available", g_ota_status.update_available);
    cJSON_AddStringToObject(root, "uptime", uptime_str);

    cJSON *wifi_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(wifi_obj, "ssid", wifi_ctx->ssid);
    cJSON_AddNumberToObject(wifi_obj, "rssi", wifi_ctx->rssi);
    cJSON_AddStringToObject(wifi_obj, "state", wifi_ctx->state == WIFI_CONNECTED ? "connected" : "disconnected");

    if (wifi_ctx->ip.has_ip)
    {
        cJSON_AddStringToObject(wifi_obj, "ip", wifi_ctx->ip.ip_str);
    }
    else if (wifi_ctx->ip6.has_ip)
    {
        cJSON_AddStringToObject(wifi_obj, "ip", wifi_ctx->ip6.ip_str);
    }
    else
    {
        cJSON_AddStringToObject(wifi_obj, "ip", "unknown");
    }

    cJSON_AddItemToObject(root, "wifi", wifi_obj);

    char *response = cJSON_Print(root);
    if (!response)
    {
        ESP_LOGE("HTTP", "Failed to print JSON");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    // ESP_LOGI("HTTP", "Sending response: %s", response);
    esp_err_t ret = httpd_resp_sendstr(req, response);

    if (ret != ESP_OK)
    {
        ESP_LOGE("HTTP", "Failed to send response: %d", ret);
    }

    free(response);
    cJSON_Delete(root);
    return ret;
}

static esp_err_t handle_ota_update(httpd_req_t *req)
{
    esp_ota_handle_t ota_handle;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);

    if (update_partition == NULL)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No OTA partition found");
        return ESP_FAIL;
    }

    // Buffer for storing firmware data
    uint8_t *firmware_buffer = malloc(req->content_len);
    if (!firmware_buffer)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    // Read entire firmware into buffer
    int total_len = 0;
    int recv_len = 0;
    while (total_len < req->content_len)
    {
        recv_len = httpd_req_recv(req, (char *)firmware_buffer + total_len,
                                  req->content_len - total_len);
        if (recv_len <= 0)
        {
            free(firmware_buffer);
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Receive failed");
            return ESP_FAIL;
        }
        total_len += recv_len;
    }

    // Verify signature
    if (verify_firmware_signature(firmware_buffer, req->content_len - 512,
                                  firmware_buffer + req->content_len - 512) != ESP_OK)
    {
        free(firmware_buffer);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid signature");
        return ESP_FAIL;
    }

    // Start OTA
    esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
    if (err != ESP_OK)
    {
        free(firmware_buffer);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA begin failed");
        return ESP_FAIL;
    }

    // Write firmware (excluding signature)
    err = esp_ota_write(ota_handle, firmware_buffer, req->content_len - 512);
    free(firmware_buffer);

    if (err != ESP_OK)
    {
        esp_ota_abort(ota_handle);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Firmware write failed");
        return ESP_FAIL;
    }

    err = esp_ota_end(ota_handle);
    if (err != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA end failed");
        return ESP_FAIL;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to set boot partition");
        return ESP_FAIL;
    }

    httpd_resp_sendstr(req, "Update successful. Rebooting...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();

    return ESP_OK;
}

esp_err_t check_for_update(void)
{
    esp_http_client_config_t config = {
        .url = UPDATE_CHECK_URL,
        .method = HTTP_GET,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        char *buffer = malloc(esp_http_client_get_content_length(client) + 1);
        esp_http_client_read_response(client, buffer, esp_http_client_get_content_length(client));
        buffer[esp_http_client_get_content_length(client)] = '\0';

        cJSON *root = cJSON_Parse(buffer);
        const char *version = cJSON_GetObjectItem(root, "version")->valuestring;
        const char *signature_b64 = cJSON_GetObjectItem(root, "signature")->valuestring;

        size_t sig_len;
        mbedtls_base64_decode(g_ota_status.signature,
                              sizeof(g_ota_status.signature),
                              &sig_len,
                              (const unsigned char *)signature_b64,
                              strlen(signature_b64));

        free(buffer);
        g_ota_status.latest_version = strdup(version);
        g_ota_status.update_available = strcmp(version, g_ota_status.current_version) > 0;

        cJSON_Delete(root);
    }

    esp_http_client_cleanup(client);
    return err;
}

static esp_err_t handle_options_status(httpd_req_t *req)
{
    ESP_LOGI("HTTP", "Handling OPTIONS request for /status");

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type, Accept");
    httpd_resp_set_hdr(req, "Access-Control-Max-Age", "3600");

    // For OPTIONS, we send 204 No Content
    httpd_resp_set_status(req, "204 No Content");
    httpd_resp_sendstr(req, "");

    return ESP_OK;
}

static esp_err_t handle_options_update(httpd_req_t *req)
{
    ESP_LOGI("HTTP", "Handling OPTIONS request for /update");

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    httpd_resp_set_hdr(req, "Access-Control-Max-Age", "3600");

    httpd_resp_set_status(req, "204 No Content");
    httpd_resp_sendstr(req, "");

    return ESP_OK;
}

esp_err_t init_ota_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = OTA_SERVER_PORT;

    if (httpd_start(&server, &config) != ESP_OK)
    {
        return ESP_FAIL;
    }

    // Register OPTIONS handlers
    httpd_uri_t status_options = {
        .uri = "/status",
        .method = HTTP_OPTIONS,
        .handler = handle_options_status,
        .user_ctx = NULL};

    httpd_uri_t update_options = {
        .uri = "/update",
        .method = HTTP_OPTIONS,
        .handler = handle_options_update,
        .user_ctx = NULL};

    // Register existing handlers
    httpd_uri_t status_uri = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = handle_get_status,
        .user_ctx = NULL};

    httpd_uri_t ota_update_uri = {
        .uri = "/update",
        .method = HTTP_POST,
        .handler = handle_ota_update,
        .user_ctx = NULL};

    // Register all handlers - order matters! OPTIONS first
    httpd_register_uri_handler(server, &status_options);
    httpd_register_uri_handler(server, &update_options);
    httpd_register_uri_handler(server, &status_uri);
    httpd_register_uri_handler(server, &ota_update_uri);

    return ESP_OK;
}