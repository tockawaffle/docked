#include "sd_card.h"
#include "lv_fs.h"

static sdmmc_card_t *card;
static const char mount_point[] = MOUNT_POINT;
static sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static esp_err_t s_example_write_file(const char *path, char *data)
{
    ESP_LOGI(SD_TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        ESP_LOGE(SD_TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(SD_TAG, "File written");

    return ESP_OK;
}

/**
 * @brief Initializes the SD card.
 *
 * This function initializes the SD card by performing the following steps:
 * 1. Reads the current state from the CH422G command register using I2C.
 * 2. Writes to the CH422G data register using I2C.
 * 3. Initializes the SPI bus with the specified configuration.
 * 4. Configures the SD card slot.
 * 5. Mounts the filesystem on the SD card.
 * 6. Reads and logs the SD card information.
 *
 * @return
 *     - ESP_OK: Success
 *     - ESP_FAIL: Failure
 *     - Other error codes from I2C, SPI, or SD card operations
 */
esp_err_t sd_card_init(void)
{
    esp_err_t ret;
    ESP_LOGI(SD_TAG, "Starting SD card initialization");

    // Use existing I2C to control CH422G
    // Command register write
    uint8_t current_state; // Read current state
    ret = i2c_master_read_from_device(I2C_MASTER_NUM, 0x38, &current_state, 1,
                                      I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SD_TAG, "Failed to write to CH422G command register: %s", esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(10));

    // Data register write
    uint8_t write_buf = 0x01;
    ret = i2c_master_write_to_device(I2C_MASTER_NUM, 0x24, &write_buf, 1,
                                     I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SD_TAG, "Failed to write to CH422G data register: %s", esp_err_to_name(ret));
        return ret;
    }
    vTaskDelay(pdMS_TO_TICKS(10));

    // Initialize SPI bus
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ESP_LOGI(SD_TAG, "Initializing SPI bus (MOSI:%d, MISO:%d, CLK:%d)",
             PIN_NUM_MOSI, PIN_NUM_MISO, PIN_NUM_CLK);

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SD_TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure SD card slot
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = -1; // CS controlled by CH422G
    slot_config.host_id = host.slot;

    /**
     * @brief Mounts the filesystem on the SD card.
     *
     * This function initializes the SD card and mounts the filesystem,
     * making the SD card ready for read and write operations.
     */
    // Mount filesystem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024};

    ESP_LOGI(SD_TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SD_TAG, "Failed to mount filesystem: %s", esp_err_to_name(ret));
        spi_bus_free(host.slot);
        return ret;
    }

    if (card == NULL)
    {
        ESP_LOGE(SD_TAG, "Card mount succeeded but card is NULL!");
        esp_vfs_fat_sdcard_unmount(mount_point, card);
        spi_bus_free(host.slot);
        return ESP_FAIL;
    }

    // Try to read card info
    ESP_LOGI(SD_TAG, "Card info:");
    ESP_LOGI(SD_TAG, "Name: %s", card->cid.name);
    ESP_LOGI(SD_TAG, "Capacity: %lluMB", ((uint64_t)card->csd.capacity) * card->csd.sector_size / (1024 * 1024));
    ESP_LOGI(SD_TAG, "Sector size: %d", card->csd.sector_size);

    ESP_LOGI(SD_TAG, "SD Card mounted successfully");

    ESP_LOGI(SD_TAG, "Initializing LittlevGL file system interface");
    ret = init_lvgl_fs();
    if (ret != ESP_OK)
    {
        ESP_LOGE(SD_TAG, "Failed to initialize LittlevGL file system interface: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(SD_TAG, "LittlevGL file system interface initialized");

    return ESP_OK;
}

/**
 * @brief Get the SD card object.
 *
 * This function returns a pointer to the sdmmc_card_t structure representing
 * the SD card.
 *
 * @return sdmmc_card_t* Pointer to the SD card object.
 */
sdmmc_card_t *get_sd_card(void)
{
    return card;
}