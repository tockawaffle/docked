#pragma once

#include <esp_err.h>
#include <lvgl.h>
#include "splash_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and display the splash screen
 * @param main_screen The main LVGL screen object
 */
void splash_screen_init(lv_obj_t *main_screen);

/**
 * @brief Get the current splash screen context
 * @return Pointer to the splash context structure
 */
splash_ctx_t *splash_screen_get_context(void);

#ifdef __cplusplus
}
#endif