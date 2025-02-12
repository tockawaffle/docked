#ifndef STORAGE_TYPES_H
#define STORAGE_TYPES_H

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#include <dirent.h>
#include "lcd_port.h"

#define SD_TAG "SDCard"

#define MOUNT_POINT "/sdcard" // Mount point for the SD card

#define I2C_MASTER_NUM_SD 1    /*!< I2C port for SD card */
#define I2C_MASTER_SDA_IO_SD 6 /*!< GPIO for I2C SDA for SD card */
#define I2C_MASTER_SCL_IO_SD 7 /*!< GPIO for I2C SCL for SD card */

// Pin assignments for SD SPI interface
#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK 12

/**
 * @brief Initialize the SD card.
 *
 * This function initializes the SD card and prepares it for use.
 *
 * @return
 *     - ESP_OK: Success
 *     - ESP_FAIL: Failed to initialize the SD card
 */
 
/**
 * @brief Get the SD card information.
 *
 * This function returns a pointer to the sdmmc_card_t structure that contains
 * information about the SD card.
 *
 * @return
 *     - sdmmc_card_t*: Pointer to the SD card information structure
 */
esp_err_t sd_card_init(void);
/**
 * @brief Retrieve the SD card instance.
 *
 * This function returns a pointer to the sdmmc_card_t structure representing
 * the SD card instance. It can be used to access various properties and 
 * perform operations on the SD card.
 *
 * @return Pointer to the sdmmc_card_t structure representing the SD card.
 */
sdmmc_card_t *get_sd_card(void);

#endif /* STORAGE_TYPES_H */