#include "wifi_storage_types.h"

#define STORAGE_NAMESPACE "wifi_config"
#define TAG "WiFiConfig"

static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

esp_err_t save_wifi_config(const char *ssid, const char *password)
{
    if (!ssid || !password) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Saving WiFi credentials for SSID: %s", ssid);

    // Initialize NVS
    esp_err_t ret = init_nvs();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    // Open NVS handle
    nvs_handle_t handle;
    ret = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // Save the credentials as binary blobs to preserve encoding
    size_t ssid_len = strlen(ssid);
    size_t pass_len = strlen(password);

    // Save SSID
    ret = nvs_set_blob(handle, "ssid", ssid, ssid_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save SSID: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    // Save SSID length
    ret = nvs_set_u32(handle, "ssid_len", ssid_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save SSID length: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    // Save password
    ret = nvs_set_blob(handle, "pass", password, pass_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save password: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    // Save password length
    ret = nvs_set_u32(handle, "pass_len", pass_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save password length: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    // Commit changes
    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit to NVS: %s", esp_err_to_name(ret));
    }

    nvs_close(handle);
    return ret;
}

esp_err_t read_wifi_config(wifi_credentials_t *credentials)
{
    if (!credentials) {
        return ESP_ERR_INVALID_ARG;
    }

    // Clear the credentials structure
    memset(credentials, 0, sizeof(wifi_credentials_t));

    // Initialize NVS
    esp_err_t ret = init_nvs();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    // Open NVS handle
    nvs_handle_t handle;
    ret = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // Read lengths first
    uint32_t ssid_len = 0;
    uint32_t pass_len = 0;

    ret = nvs_get_u32(handle, "ssid_len", &ssid_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read SSID length: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    ret = nvs_get_u32(handle, "pass_len", &pass_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read password length: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    // Validate lengths
    if (ssid_len >= MAX_SSID_LENGTH || pass_len >= MAX_PASSWORD_LENGTH) {
        ESP_LOGE(TAG, "Stored credential lengths exceed maximum");
        nvs_close(handle);
        return ESP_ERR_INVALID_SIZE;
    }

    // Read SSID
    size_t read_ssid_len = ssid_len;
    ret = nvs_get_blob(handle, "ssid", credentials->ssid, &read_ssid_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read SSID: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    credentials->ssid[read_ssid_len] = '\0';

    // Read password
    size_t read_pass_len = pass_len;
    ret = nvs_get_blob(handle, "pass", credentials->password, &read_pass_len);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read password: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }
    credentials->password[read_pass_len] = '\0';

    ESP_LOGI(TAG, "Successfully read WiFi credentials for SSID: %s", credentials->ssid);
    nvs_close(handle);
    return ESP_OK;
}

esp_err_t remove_wifi_config(void)
{
    // Initialize NVS
    esp_err_t ret = init_nvs();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    // Open NVS handle
    nvs_handle_t handle;
    ret = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(ret));
        return ret;
    }

    // Erase all keys in the namespace
    ret = nvs_erase_all(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase credentials: %s", esp_err_to_name(ret));
        nvs_close(handle);
        return ret;
    }

    // Commit the change
    ret = nvs_commit(handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit to NVS: %s", esp_err_to_name(ret));
    }

    nvs_close(handle);
    return ret;
}