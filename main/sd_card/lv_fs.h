#ifndef LVGL_FS_H
#define LVGL_FS_H

#include <esp_err.h>
#include "lvgl.h"
#include "sd_card.h"
#include <errno.h>
/**
 * @brief Initialize the LVGL filesystem driver for SD card access
 * 
 * This function initializes a filesystem driver for LVGL that provides access
 * to files stored on the SD card. It uses the drive letter 'S:' for all operations.
 * The function will wait up to 5 seconds for the SD card to become available.
 * 
 * Usage example:
 *   esp_err_t ret = init_lvgl_fs();
 *   if (ret != ESP_OK) {
 *       // Handle error
 *   }
 *   // Use LVGL file operations with "S:/path/to/file"
 * 
 * @return ESP_OK if initialization was successful
 *         ESP_ERR_TIMEOUT if SD card was not available after timeout
 *         ESP_ERR_INVALID_STATE if LVGL is not initialized
 */
esp_err_t init_lvgl_fs(void);

// Configuration constants that can be modified if needed
#ifndef LVGL_FS_LETTER
#define LVGL_FS_LETTER 'S'         /**< Drive letter used for the SD card */
#endif

#ifndef LVGL_FS_MAX_PATH_LEN
#define LVGL_FS_MAX_PATH_LEN 256   /**< Maximum length for file paths */
#endif

#ifndef LVGL_FS_MOUNT_POINT
#define LVGL_FS_MOUNT_POINT "/sdcard"  /**< Mount point for the SD card */
#endif

#ifndef LVGL_FS_TIMEOUT_MS
#define LVGL_FS_TIMEOUT_MS 5000     /**< Timeout in ms to wait for SD card */
#endif

#ifndef LVGL_FS_CHECK_INTERVAL_MS
#define LVGL_FS_CHECK_INTERVAL_MS 100 /**< Interval in ms between SD card checks */
#endif

#endif /* LVGL_FS_H */