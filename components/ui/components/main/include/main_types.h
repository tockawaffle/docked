#pragma once

#include <lvgl.h>
#include <stdint.h>
#include <stdbool.h>
#include "wifi_c_types.h"

// Version control for ABI compatibility
#define SIDEBAR_API_VERSION 1

// Security bounds
#define MAX_MACRO_PAGES 5
#define MAX_BUTTONS 15
#define MAX_BUTTON_TEXT_LENGTH 32
#define MAX_SCENE_NAME_LENGTH 32

// Sidebar dimensions
#define SIDEBAR_WIDTH 145
#define SIDEBAR_BUTTON_WIDTH LV_PCT(80)
#define ICON_SIZE 48
#define LOGO_SIZE 50
#define MENU_WIDTH 240
#define MENU_ANIM_TIME 200

typedef uint8_t sidebar_icon_type_t;
#define SIDEBAR_ICON_SYMBOL ((sidebar_icon_type_t)0)
#define SIDEBAR_ICON_IMAGE ((sidebar_icon_type_t)1)

// Validation macros
#define IS_VALID_ICON_TYPE(type) ((type) == SIDEBAR_ICON_SYMBOL || (type) == SIDEBAR_ICON_IMAGE)
#define IS_VALID_WIFI_STATE(state) ((state) == WIFI_CONNECTED || (state) == WIFI_DISCONNECTED)
#define IS_VALID_TEXT_LENGTH(len) ((len) <= MAX_BUTTON_TEXT_LENGTH)

typedef struct
{
    const char *text;
    size_t text_length;
    sidebar_icon_type_t icon_type;
    union
    {
        const char *symbol;
        const lv_img_dsc_t *image;
    } icon;
    lv_event_cb_t callback;
    uint32_t checksum;
} sidebar_btn_t;

typedef struct
{
    struct
    {
        volatile wifi_state_t state;
        volatile char name[33];
        volatile int8_t signal_strength;
    } wifi;

    lv_obj_t *main_screen;
    lv_obj_t *sidebar;
    lv_obj_t *active_btn;
    lv_obj_t *wifi_indicator;
    uint32_t button_count;
    uint32_t checksum;
} sidebar_ctx_t;

typedef struct
{
    lv_obj_t *menu_cont;
    lv_obj_t *current_menu;
    bool is_open;
} menu_ctx_t;