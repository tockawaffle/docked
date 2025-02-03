#pragma once

#include <esp_err.h>
#include <lvgl.h>
#include "main_types.h"
#include "menu_types.h"

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

    static sidebar_ctx_t ctx = {
        .button_count = 0,
        .wifi = {
            .signal_strength = 0,
            .name = "",
            .state = WIFI_DISCONNECTED},
        .active_btn = NULL,
        .checksum = 0};

    static const menu_def_t home_menu = {
        .title = "Home",
        .items = (menu_item_t[]){
            {"Macros", NULL},
            {"Scenes", NULL},
            {"Audio", NULL}},
        .item_count = 3};

    static const menu_def_t settings_menu = {
        .title = "Settings",
        .items = (menu_item_t[]){
            {"General", NULL},
            {"Network", NULL},
            {"Display", NULL},
            {"System", NULL}},
        .item_count = 4};

    static menu_ctx_t menu_ctx = {
        .menu_cont = NULL,
        .current_menu = NULL,
        .is_open = false};

    struct wifi_state_style
    {
        const char *symbol;
        lv_color_t color;
    };

    static struct wifi_state_style WIFI_STATES[2];

#ifdef __cplusplus
}
#endif