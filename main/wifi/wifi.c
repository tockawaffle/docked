#include "wifi.h"

static EventGroupHandle_t s_wifi_event_group = NULL;

static esp_netif_t *wifi_netif = NULL;
static esp_event_handler_instance_t ip_event_handler;
static esp_event_handler_instance_t wifi_event_handler;

static const int WIFI_RETRY_ATTEMPT = 3;
static int wifi_retry_count = 0;
static uint8_t last_disconnect_reason = 0;

static void ip_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(WIFI_TAG, "Handling IP event, event code 0x%" PRIx32, event_id);
    switch (event_id)
    {
    case (IP_EVENT_STA_GOT_IP):
        ip_event_got_ip_t *event_ip = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&event_ip->ip_info.ip));
        wifi_retry_count = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case (IP_EVENT_STA_LOST_IP):
        ESP_LOGI(WIFI_TAG, "Lost IP");
        break;
    case (IP_EVENT_GOT_IP6):
        ip_event_got_ip6_t *event_ip6 = (ip_event_got_ip6_t *)event_data;
        ESP_LOGI(WIFI_TAG, "Got IPv6: " IPV6STR, IPV62STR(event_ip6->ip6_info.ip));
        wifi_retry_count = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        ESP_LOGI(WIFI_TAG, "IP event not handled");
        break;
    }
}

static const char *get_detailed_wifi_reason(uint8_t reason)
{
    switch (reason)
    {
    // Standard IEEE reasons
    case 1:
        return "Internal failure";
    case 2:
        return "Authentication expired or is no longer valid";
    case 3:
        return "Device is leaving or has left the network";
    case 4:
        return "Disconnected due to inactivity";
    case 5:
        return "AP cannot handle all connected devices";
    case 6:
        return "Class 2 frame received from non-authenticated device";
    case 7:
        return "Class 3 frame received from non-associated device";
    case 8:
        return "Device left network";
    case 9:
        return "Device requesting association is not authenticated";
    case 10:
        return "Power capabilities are unacceptable";
    case 11:
        return "Supported channels are unacceptable";
    case 12:
        return "BSS transition management";
    case 13:
        return "Invalid element in frame";
    case 14:
        return "Message integrity check (MIC) failure";
    case 15:
        return "4-way handshake timeout";
    case 16:
        return "Group key handshake timeout";
    case 17:
        return "Handshake element mismatch";
    case 18:
        return "Invalid group cipher";
    case 19:
        return "Invalid pairwise cipher";
    case 20:
        return "Invalid AKMP";
    case 21:
        return "Unsupported RSN IE version";
    case 22:
        return "Invalid RSN IE capabilities";
    case 23:
        return "IEEE 802.1X authentication failed";
    case 24:
        return "Cipher suite rejected by policy";

    // Espressif specific reasons
    case 200:
        return "Lost connection - beacon timeout";
    case 201:
        return "No AP found matching requirements";
    case 202:
        return "Authentication failed";
    case 203:
        return "Association failed";
    case 204:
        return "Handshake failed";
    case 205:
        return "Connection to AP failed";
    case 206:
        return "AP TSF reset occurred";
    case 207:
        return "Roaming to another AP";
    case 208:
        return "Association comeback time too long";
    case 209:
        return "AP did not reply to SA query";
    case 210:
        return "No AP found with compatible security settings";
    case 211:
        return "No AP found meeting authentication mode requirements";
    case 212:
        return "No AP found meeting RSSI requirements";

    default:
        return "Unknown reason";
    }
}

static const char *get_security_specific_reason(uint8_t reason)
{
    if (reason == 210)
    { // NO_AP_FOUND_SECURITY
        return "Security configuration mismatch. Possible causes:\n"
               "- WEP/Open mode mismatch\n"
               "- Enterprise security mismatch\n"
               "- SAE-PK/H2E support mismatch\n"
               "- PMF capability mismatch\n"
               "- Cipher compatibility issues\n"
               "- OWE mode mismatch";
    }
    return NULL;
}

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
        last_disconnect_reason = 0;
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        wifi_event_sta_disconnected_t *disconn = (wifi_event_sta_disconnected_t *)event_data;
        const char *reason_str = get_detailed_wifi_reason(disconn->reason);
        const char *security_detail = get_security_specific_reason(disconn->reason);

        ESP_LOGI(WIFI_TAG, "Wi-Fi disconnected, reason: %d (%s)",
                 disconn->reason, reason_str);

        if (security_detail)
        {
            ESP_LOGI(WIFI_TAG, "Security details: %s", security_detail);
        }

        last_disconnect_reason = disconn->reason;

        if (wifi_retry_count < WIFI_RETRY_ATTEMPT)
        {
            ESP_LOGI(WIFI_TAG, "Retrying to connect... (attempt %d/%d)",
                     wifi_retry_count + 1, WIFI_RETRY_ATTEMPT);
            esp_wifi_connect();
            wifi_retry_count++;
        }
        else
        {
            ESP_LOGE(WIFI_TAG, "Failed to connect after %d attempts",
                     WIFI_RETRY_ATTEMPT);
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        break;
    }
}

esp_err_t wifi_init(void)
{
    // Initialize Non-Volatile Storage (NVS)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    s_wifi_event_group = xEventGroupCreate();

    ret = esp_netif_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "Failed to initialize TCP/IP network stack");
        return ret;
    }

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "Failed to create default event loop");
        return ret;
    }

    ret = esp_wifi_set_default_wifi_sta_handlers();
    if (ret != ESP_OK)
    {
        ESP_LOGE(WIFI_TAG, "Failed to set default handlers");
        return ret;
    }

    wifi_netif = esp_netif_create_default_wifi_sta();
    if (wifi_netif == NULL)
    {
        ESP_LOGE(WIFI_TAG, "Failed to create default WiFi STA interface");
        return ESP_FAIL;
    }

    // Wi-Fi stack configuration parameters
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_netif_set_hostname(wifi_netif, "decksterity");

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_cb,
                                                        NULL,
                                                        &wifi_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &ip_event_cb,
                                                        NULL,
                                                        &ip_event_handler));
    return ret;
}

wifi_detailed_status_t wifi_connect(char *wifi_ssid, char *wifi_password)
{
    wifi_detailed_status_t status = {
        .code = ESP_OK,
        .message = "Success",
        .reason_code = 0,
        .reason_str = "No error"};

    // Reset retry counter
    wifi_retry_count = 0;

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

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(WIFI_TAG, "Connecting to network %s", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdTRUE,
                                           pdFALSE,
                                           pdMS_TO_TICKS(30000));

    if (bits & WIFI_CONNECTED_BIT)
    {
        status.code = ESP_OK;
        status.message = "Successfully connected to WiFi";
        status.reason_code = 0;
        status.reason_str = "Connection successful";
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        status.code = ESP_FAIL;
        status.reason_code = last_disconnect_reason;
        status.reason_str = get_detailed_wifi_reason(last_disconnect_reason);

        // Provide more detailed error messages based on reason code
        switch (last_disconnect_reason)
        {
        case 1: // UNSPECIFIED
            status.message = "Connection failed due to internal error";
            break;
        case 2: // AUTH_EXPIRE
            status.message = "Authentication expired - please try reconnecting";
            break;
        case 15:  // 4WAY_HANDSHAKE_TIMEOUT
        case 204: // HANDSHAKE_TIMEOUT
            status.message = "Security handshake failed - check password";
            break;
        case 200: // BEACON_TIMEOUT
            status.message = "Lost connection to AP - weak signal or AP unavailable";
            break;
        case 201: // NO_AP_FOUND
            status.message = "Network not found - check SSID";
            break;
        case 202: // AUTH_FAIL
            status.message = "Authentication failed - verify credentials";
            break;
        case 203: // ASSOC_FAIL
            status.message = "Association failed - AP rejected connection";
            break;
        case 210: // NO_AP_FOUND_SECURITY
            status.message = "Security configuration mismatch";
            break;
        case 211: // NO_AP_FOUND_AUTHMODE
            status.message = "No AP found with required security level";
            break;
        case 212: // NO_AP_FOUND_RSSI
            status.message = "No AP found with sufficient signal strength";
            break;
        default:
            status.message = "Connection failed";
            break;
        }
    }
    else
    {
        status.code = ESP_ERR_TIMEOUT;
        status.message = "Connection timeout";
        status.reason_code = 0;
        status.reason_str = "Timeout waiting for connection";
    }

    return status;
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
    if (ret == ESP_ERR_WIFI_NOT_INIT)
    {
        ESP_LOGE(WIFI_TAG, "Wi-Fi stack not initialized");
        return ret;
    }

    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));
    esp_netif_destroy(wifi_netif);

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, ESP_EVENT_ANY_ID, ip_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler));

    return ESP_OK;
}

esp_err_t wifi_restart(void)
{
    esp_err_t ret = esp_wifi_stop();
    if (ret == ESP_ERR_WIFI_NOT_INIT)
    {
        ESP_LOGE(WIFI_TAG, "Wi-Fi stack not initialized");
        return ret;
    }

    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));
    esp_netif_destroy(wifi_netif);

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, ESP_EVENT_ANY_ID, ip_event_handler));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler));

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

get_wifi_t get_wifi(void)
{
    get_wifi_t result;
    result.status = ESP_OK;
    result.current_state = WIFI_DISCONNECTED;
    memset(result.network, 0, sizeof(result.network));

    wifi_ap_record_t ap_info;

    // Get current WiFi state
    wifi_mode_t mode;
    result.status = esp_wifi_get_mode(&mode);
    if (result.status != ESP_OK)
    {
        return result;
    }

    // Check if WiFi is in station mode
    if (mode != WIFI_MODE_STA)
    {
        return result;
    }

    // Check if we're connected
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
    {
        result.current_state = WIFI_CONNECTED;
        strncpy(result.network, (char *)ap_info.ssid, sizeof(result.network) - 1);
        result.network[sizeof(result.network) - 1] = '\0';
    }

    return result;
}