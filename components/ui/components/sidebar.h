#ifndef SIDEBAR_H
#define SIDEBAR_H

#include "lvgl.h"

// Main sidebar structure
typedef struct {
    lv_obj_t *sidebar;
    lv_obj_t *toggle_btn;
    lv_obj_t *logo_container;
    lv_obj_t *buttons_container;
    bool is_open;
    uint16_t open_width;
    uint16_t closed_width;
} sidebar_t;

// Button data structure
typedef struct {
    const char *icon;           // LVGL symbol or image source
    const char *label;          // Button text
    lv_event_cb_t callback;     // Click handler
    lv_color_t icon_color;      // Optional icon color
} sidebar_button_t;

// Function declarations
sidebar_t* create_sidebar(lv_obj_t *parent);

#endif /* SIDEBAR_H */