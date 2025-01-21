#pragma once

#include <esp_err.h>
#include <lvgl.h>
#include "splash_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Initialize and display the splash screen
     * @param main_screen The main LVGL screen object
     */
    void splash_screen_init();

    /**
     * @brief Get the current splash screen context
     * @return Pointer to the splash context structure
     */
    splash_ctx_t *splash_screen_get_context(void);

    /**
     * @brief Set the current splash screen state
     * @param new_ctx The new state to set
     * @return The previous state
     */
    splash_init_state_t splash_screen_set_state(splash_init_state_t new_ctx);

    /**
     * @brief Delete a timer and log the caller function
     * @param timer The timer to delete
     * @param caller_function The name of the function that is deleting the timer
     */
    void splash_delete_timer(lv_timer_t *timer, const char *caller_function);

#ifdef __cplusplus
}
#endif