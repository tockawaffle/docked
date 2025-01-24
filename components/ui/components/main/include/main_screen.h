#pragma once

#include <esp_err.h>
#include <lvgl.h>
#include "main_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize and display the main screen
     */
    void main_screen_init();

    /**
     * @brief Creates a separator
     */
    lv_obj_t *separator(lv_obj_t *parent, uint8_t width_pct, lv_coord_t pad_top, lv_coord_t pad_bottom);

#ifdef __cplusplus
}
#endif