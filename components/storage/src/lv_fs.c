#include "storage_internal.h"

#define FS_TAG "LVGL_FS"
#define FS_LETTER 'S'         // Drive letter for SD card
#define MAX_PATH_LEN 256      // Maximum path length
#define MOUNT_POINT "/sdcard" // SD card mount point

typedef struct
{
    FILE *file;
    char path[MAX_PATH_LEN];
} fs_file_t;

// Error translation helper
static lv_fs_res_t translate_ferror(FILE *file)
{
    if (file == NULL)
        return LV_FS_RES_UNKNOWN;

    int error = ferror(file);
    clearerr(file); // Clear the error state

    switch (error)
    {
    case 0:
        return LV_FS_RES_OK;
    default:
    {
        // Get the errno value
        if (errno == 0)
            return LV_FS_RES_UNKNOWN;

        switch (errno)
        {
        case ENOENT:
            return LV_FS_RES_NOT_EX; // No such file or directory
        case EACCES:
            return LV_FS_RES_DENIED; // Permission denied
        case EEXIST:
            return LV_FS_RES_DENIED; // File exists
        case ENOSPC:
            return LV_FS_RES_FULL; // No space left on device
        case ENOMEM:
            return LV_FS_RES_OUT_OF_MEM; // Out of memory
        case EINVAL:
            return LV_FS_RES_INV_PARAM; // Invalid argument
        default:
            return LV_FS_RES_UNKNOWN;
        }
    }
    }
}

// Path construction helper
static esp_err_t construct_path(char *full_path, size_t max_len, const char *path)
{
    if (path == NULL || full_path == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Skip drive letter if present (S:/)
    if (path[0] == FS_LETTER && path[1] == ':')
    {
        path += 2;
    }
    if (path[0] == '/')
    {
        path++;
    }

    int written = snprintf(full_path, max_len, "%s/%s", MOUNT_POINT, path);
    if (written < 0 || written >= max_len)
    {
        ESP_LOGE(FS_TAG, "Path too long: %s", path);
        return ESP_ERR_INVALID_ARG;
    }

    return ESP_OK;
}

static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    fs_file_t *file_p = malloc(sizeof(fs_file_t));
    if (file_p == NULL)
    {
        ESP_LOGE(FS_TAG, "Failed to allocate file handle");
        return NULL;
    }

    char full_path[MAX_PATH_LEN];
    if (construct_path(full_path, sizeof(full_path), path) != ESP_OK)
    {
        free(file_p);
        return NULL;
    }

    const char *flags = "";
    if (mode == LV_FS_MODE_WR)
        flags = "wb";
    else if (mode == LV_FS_MODE_RD)
        flags = "rb";
    else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
        flags = "rb+";

    file_p->file = fopen(full_path, flags);
    if (file_p->file == NULL)
    {
        ESP_LOGE(FS_TAG, "Failed to open file: %s (errno: %d)", full_path, errno);
        free(file_p);
        return NULL;
    }

    strncpy(file_p->path, full_path, MAX_PATH_LEN - 1);
    file_p->path[MAX_PATH_LEN - 1] = '\0';

    ESP_LOGI(FS_TAG, "Opened file: %s", file_p->path);
    return file_p;
}

static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p)
{
    fs_file_t *fp = (fs_file_t *)file_p;
    if (fp == NULL)
        return LV_FS_RES_INV_PARAM;

    ESP_LOGI(FS_TAG, "Closing file: %s", fp->path);

    lv_fs_res_t res = LV_FS_RES_OK;
    if (fp->file)
    {
        if (fclose(fp->file) != 0)
        {
            ESP_LOGE(FS_TAG, "Error closing file: %s (errno: %d)", fp->path, errno);
            res = LV_FS_RES_UNKNOWN;
        }
    }

    free(fp);
    return res;
}

static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    fs_file_t *fp = (fs_file_t *)file_p;
    if (fp == NULL || fp->file == NULL || buf == NULL)
        return LV_FS_RES_INV_PARAM;

    *br = fread(buf, 1, btr, fp->file);

    if (*br != btr && !feof(fp->file))
    {
        ESP_LOGE(FS_TAG, "Error reading file: %s (errno: %d)", fp->path, errno);
        return translate_ferror(fp->file);
    }

    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    fs_file_t *fp = (fs_file_t *)file_p;
    if (fp == NULL || fp->file == NULL || buf == NULL)
        return LV_FS_RES_INV_PARAM;

    *bw = fwrite(buf, 1, btw, fp->file);

    if (*bw != btw)
    {
        ESP_LOGE(FS_TAG, "Error writing file: %s (errno: %d)", fp->path, errno);
        return translate_ferror(fp->file);
    }

    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    fs_file_t *fp = (fs_file_t *)file_p;
    if (fp == NULL || fp->file == NULL)
        return LV_FS_RES_INV_PARAM;

    int w;
    switch (whence)
    {
    case LV_FS_SEEK_SET:
        w = SEEK_SET;
        break;
    case LV_FS_SEEK_CUR:
        w = SEEK_CUR;
        break;
    case LV_FS_SEEK_END:
        w = SEEK_END;
        break;
    default:
        return LV_FS_RES_INV_PARAM;
    }

    if (fseek(fp->file, pos, w) != 0)
    {
        ESP_LOGE(FS_TAG, "Seek error in file: %s (errno: %d)", fp->path, errno);
        return translate_ferror(fp->file);
    }

    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    fs_file_t *fp = (fs_file_t *)file_p;
    if (fp == NULL || fp->file == NULL || pos_p == NULL)
        return LV_FS_RES_INV_PARAM;

    long pos = ftell(fp->file);
    if (pos < 0)
    {
        ESP_LOGE(FS_TAG, "Tell error in file: %s (errno: %d)", fp->path, errno);
        return translate_ferror(fp->file);
    }

    *pos_p = (uint32_t)pos;
    return LV_FS_RES_OK;
}

// Check if filesystem is ready
static bool fs_ready(lv_fs_drv_t *drv)
{
    return get_sd_card() != NULL;
}

esp_err_t init_lvgl_fs(void)
{
    static lv_fs_drv_t fs_drv;

    // Wait for SD card with timeout
    const int timeout_ms = 5000;
    const int check_interval_ms = 100;
    int elapsed_ms = 0;

    while (get_sd_card() == NULL && elapsed_ms < timeout_ms)
    {
        vTaskDelay(pdMS_TO_TICKS(check_interval_ms));
        elapsed_ms += check_interval_ms;
    }

    if (get_sd_card() == NULL)
    {
        ESP_LOGE(FS_TAG, "SD card not available after %d ms timeout", timeout_ms);
        return ESP_ERR_TIMEOUT;
    }

    lv_fs_drv_init(&fs_drv);

    fs_drv.letter = FS_LETTER;
    fs_drv.cache_size = 0; // No cache by default

    fs_drv.ready_cb = fs_ready;
    fs_drv.open_cb = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb = fs_seek;
    fs_drv.tell_cb = fs_tell;

    lv_fs_drv_register(&fs_drv);

    ESP_LOGI(FS_TAG, "LVGL filesystem driver initialized for SD card at %s", MOUNT_POINT);
    return ESP_OK;
}