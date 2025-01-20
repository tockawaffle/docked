#include "wifi.h"

static EventGroupHandle_t s_wifi_event_group = NULL;

static esp_netif_t *wifi_netif = NULL;
static esp_event_handler_instance_t ip_event_handler;
static esp_event_handler_instance_t wifi_event_handler;

static const int WIFI_RETRY_ATTEMPT = 3;
static int wifi_retry_count = 0;

static void wifi_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(WIFI_TAG, "Handling Wi-Fi event, event code 0x%" PRIx32, event_id);

    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(WIFI_TAG, "Wi-Fi started, connecting to AP...");
        esp_wifi_connect();
        break;

    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(WIFI_TAG, "Wi-Fi connected to AP");
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        ESP_LOGI(WIFI_TAG, "Wi-Fi disconnected, reason: %d", ((wifi_event_sta_disconnected_t *)event_data)->reason);
        if (wifi_retry_count < WIFI_RETRY_ATTEMPT)
        {
            ESP_LOGI(WIFI_TAG, "Retrying to connect... (attempt %d/%d)", wifi_retry_count + 1, WIFI_RETRY_ATTEMPT);
            esp_wifi_connect();
            wifi_retry_count++;
        }
        else
        {
            ESP_LOGE(WIFI_TAG, "Failed to connect after %d attempts", WIFI_RETRY_ATTEMPT);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        break;
    }
}

static void ip_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(WIFI_TAG, "Handling IP event, event code 0x%" PRIx32, event_id);

    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
        ip_event_got_ip_t *event_ip = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event_ip->ip_info.ip));
        wifi_retry_count = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;

    case IP_EVENT_STA_LOST_IP:
        ESP_LOGI(WIFI_TAG, "Lost IP");
        // Maybe set a different bit or handle reconnect here
        break;
    }
}

esp_err_t wifi_init(void)
{
    // First, ensure everything is cleaned up
    wifi_destroy();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Create event group
    s_wifi_event_group = xEventGroupCreate();

    // Initialize the TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Create default event loop if it doesn't exist
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(WIFI_TAG, "Failed to create event loop: %s", esp_err_to_name(ret));
        return ret;
    }

    // Create the wifi station interface
    wifi_netif = esp_netif_create_default_wifi_sta();
    if (wifi_netif == NULL)
    {
        ESP_LOGE(WIFI_TAG, "Failed to create network interface");
        return ESP_FAIL;
    }

    // Change the hostname
    ESP_ERROR_CHECK(esp_netif_set_hostname(wifi_netif, "decksterity"));

    // Initialize wifi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "Failed to init wifi: %s", esp_err_to_name(ret));
        return ret;
    }

    // Register event handlers
    ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_cb, NULL, &wifi_event_handler);
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "Failed to register wifi event handler: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_cb, NULL, &ip_event_handler);
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "Failed to register IP event handler: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t wifi_connect(char *wifi_ssid, char *wifi_password)
{
    // Reset retry counter
    wifi_retry_count = 0;

    // Clear any previous bits
    if (s_wifi_event_group)
    {
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
    }

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTHMODE,
        },
    };

    strncpy((char *)wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, wifi_password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_stop()); // Stop any previous WiFi activity
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(WIFI_TAG, "Connecting to network %s", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait longer for connection (30 seconds)
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdTRUE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(30000));

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(WIFI_TAG, "Successfully connected to %s", wifi_config.sta.ssid);
        return ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGE(WIFI_TAG, "Failed to connect to %s", wifi_config.sta.ssid);
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGE(WIFI_TAG, "Connection timeout");
        return ESP_ERR_TIMEOUT;
    }
}

esp_err_t wifi_disconnect(void)
{
    if (s_wifi_event_group)
    {
        vEventGroupDelete(s_wifi_event_group);
    }

    return esp_wifi_disconnect();
}

esp_err_t wifi_destroy(void)
{
    esp_err_t ret = esp_wifi_stop();
    if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_INIT)
    {
        ESP_LOGE(WIFI_TAG, "Failed to stop WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    if (wifi_netif)
    {
        ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));
        esp_netif_destroy(wifi_netif);
        wifi_netif = NULL;
    }

    ret = esp_wifi_deinit();
    if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_INIT)
    {
        ESP_LOGE(WIFI_TAG, "Failed to deinit WiFi: %s", esp_err_to_name(ret));
        return ret;
    }

    if (s_wifi_event_group)
    {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }

    return ESP_OK;
}

esp_err_t wifi_restart(void)
{
    ESP_LOGI(WIFI_TAG, "Restarting WiFi...");

    // Clean up everything
    wifi_destroy();

    // Reinitialize
    return wifi_init();
}

wifi_scan_result_t wifi_scan(void)
{
    wifi_scan_result_t result = {
        .status = ESP_OK,
        .ap_count = 0,
        .ssids = {{0}}, // Initialize arrays to zero
        .rssis = {0}};

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];

    ESP_LOGI(WIFI_TAG, "Wi-Fi scan started");

    // Set mode and start WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Start scan
    esp_err_t ret = esp_wifi_scan_start(NULL, true);
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "Failed to start Wi-Fi scan: %s", esp_err_to_name(ret));
        result.status = ret;
        return result;
    }

    // Get scan results
    ret = esp_wifi_scan_get_ap_num(&result.ap_count);
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "Failed to get number of access points: %s", esp_err_to_name(ret));
        result.status = ret;
        return result;
    }

    // Ensure we don't exceed array bounds
    if (result.ap_count > DEFAULT_SCAN_LIST_SIZE)
    {
        result.ap_count = DEFAULT_SCAN_LIST_SIZE;
    }

    ret = esp_wifi_scan_get_ap_records(&number, ap_info);
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "Failed to get AP records: %s", esp_err_to_name(ret));
        result.status = ret;
        return result;
    }

    // Copy results to our structure
    for (int i = 0; i < result.ap_count; i++)
    {
        strncpy(result.ssids[i], (char *)ap_info[i].ssid, 32);
        result.ssids[i][32] = '\0'; // Ensure null termination
        result.rssis[i] = ap_info[i].rssi;

        ESP_LOGI(WIFI_TAG, "SSID: %s", result.ssids[i]);
        ESP_LOGI(WIFI_TAG, "RSSI: %d", result.rssis[i]);
    }

    ESP_LOGI(WIFI_TAG, "Number of access points found: %d", result.ap_count);
    return result;
}