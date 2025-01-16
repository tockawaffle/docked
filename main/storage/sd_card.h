#ifndef _SD_CARD_
#define _SD_CARD_

#define SD_TAG "SDCard"

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#include "lcd_port.h"

// Mount point for the SD card
#define MOUNT_POINT "/sdcard"

#define I2C_MASTER_NUM_SD 1    /*!< I2C port for SD card */
#define I2C_MASTER_SDA_IO_SD 6 /*!< GPIO for I2C SDA for SD card */
#define I2C_MASTER_SCL_IO_SD 7 /*!< GPIO for I2C SCL for SD card */

// Pin assignments for SD SPI interface
#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_CLK 12

esp_err_t sd_card_init(void);
sdmmc_card_t *get_sd_card(void);

#endif