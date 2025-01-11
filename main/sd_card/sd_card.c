#include "sd_card.h"

static const char *TAG = "example";

sdmmc_card_t *card;
const char mount_point[] = MOUNT_POINT;

// By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
// For setting a specific frequency, use host.max_freq_khz (range 400kHz - 20MHz for SDSPI)
// Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
sdmmc_host_t host = SDSPI_HOST_DEFAULT();

static esp_err_t s_example_write_file(const char *path, char *data)
{
    // Opening file
    ESP_LOGW(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "w");
    if (f == NULL)
    {
        // Failed to open file for writing
        ESP_LOGW(TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    // Write data to file
    fprintf(f, data);
    fclose(f);
    // File written
    ESP_LOGW(TAG, "File written");

    return ESP_OK;
}

static esp_err_t s_example_read_file(const char *path)
{
    // Reading file
    ESP_LOGW(TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        // Failed to open file for reading
        ESP_LOGW(TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    // Read a line from the file
    char line[EXAMPLE_MAX_CHAR_SIZE];
    fgets(line, sizeof(line), f);
    fclose(f);

    // Strip newline
    char *pos = strchr(line, '\n');
    if (pos)
    {
        *pos = '\0';
    }
    // Read from file
    ESP_LOGW(TAG, "Read from file: '%s'", line);

    return ESP_OK;
}

/**
 * @brief i2c master initialization
 */
esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM_SD;

    // Configure I2C parameters
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,                // Set to master mode
        .sda_io_num = I2C_MASTER_SDA_IO,        // Set SDA pin
        .scl_io_num = I2C_MASTER_SCL_IO,        // Set SCL pin
        .sda_pullup_en = GPIO_PULLUP_ENABLE,    // Enable SDA pull-up
        .scl_pullup_en = GPIO_PULLUP_ENABLE,    // Enable SCL pull-up
        .master.clk_speed = I2C_MASTER_FREQ_HZ, // Set I2C clock speed
    };

    // Apply I2C configuration
    i2c_param_config(i2c_master_port, &conf);

    // Install I2C driver
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

esp_err_t i2c_detect_device(uint8_t address)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM_SD, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t waveshare_sd_card_init()
{
    esp_err_t ret;
    ESP_LOGI(TAG, "Starting SD card initialization");

    // 1. I2C Master Init with check
    ret = i2c_master_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C master init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "I2C master initialized successfully");

    // 2. Check if CH422G is responding
    ret = i2c_detect_device(0x24);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CH422G not detected at address 0x24: %s", esp_err_to_name(ret));
        
        // Try to scan I2C bus to find what devices are present
        ESP_LOGI(TAG, "Scanning I2C bus...");
        for (uint8_t i = 0x08; i < 0x78; i++) {
            ret = i2c_detect_device(i);
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Found device at address 0x%02x", i);
            }
        }
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "CH422G detected successfully");

    // 5. Mount configuration
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // 6. SD Card slot configuration
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem with CS pin: %d", PIN_NUM_CS);

    // 7. Mount filesystem
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount filesystem: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "SD card initialization complete");
    return ESP_OK;
}

esp_err_t waveshare_sd_card_test()
{
    esp_err_t ret;

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files

    // First create a file
    const char *file_hello = MOUNT_POINT "/hello.txt";
    char data[EXAMPLE_MAX_CHAR_SIZE];
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Hello", card->cid.name);
    // Write data to file
    ret = s_example_write_file(file_hello, data);
    if (ret != ESP_OK)
    {
        return ESP_FAIL;
    }

    const char *file_foo = MOUNT_POINT "/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0)
    {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    ESP_LOGW(TAG, "Renaming file %s to %sv", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0)
    {
        // Rename failed
        ESP_LOGW(TAG, "Rename failed");
        return ESP_FAIL;
    }

    // Read renamed file
    ret = s_example_read_file(file_foo);
    if (ret != ESP_OK)
    {
        return ESP_FAIL;
    }

    // Format FATFS
    ret = esp_vfs_fat_sdcard_format(mount_point, card);
    if (ret != ESP_OK)
    {
        // Failed to format FATFS
        ESP_LOGW(TAG, "Failed to format FATFS (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    // Check if file still exists after formatting
    if (stat(file_foo, &st) == 0)
    {
        ESP_LOGW(TAG, "file still exists");
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGW(TAG, "file doesnt exist, format done");
    }

    // Create a new file "nihao.txt" after formatting
    const char *file_nihao = MOUNT_POINT "/nihao.txt";
    memset(data, 0, EXAMPLE_MAX_CHAR_SIZE);                                     // Clear the data buffer
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Nihao", card->cid.name); // Writing data
    ret = s_example_write_file(file_nihao, data);                               // Writing data to a file
    if (ret != ESP_OK)
    {
        return ESP_FAIL;
    }

    // Open and read the newly created file
    ret = s_example_read_file(file_nihao);
    if (ret != ESP_OK)
    {
        return ESP_FAIL;
    }

    // All done, unmount partition and disable SPI peripheral
    esp_vfs_fat_sdcard_unmount(mount_point, card);
    ESP_LOGW(TAG, "Card unmounted");

    // Deinitialize the SPI bus after all devices are removed
    spi_bus_free(host.slot);
    return ESP_OK;
}