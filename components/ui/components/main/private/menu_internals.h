#pragma once

#include "main_internals.h"

#define MENU_TAG "Menu"

// Menu item callback function type
typedef void (*menu_item_cb_t)(void);

// Menu functions
lv_obj_t *create_slide_menu(lv_obj_t *parent, const menu_def_t *menu_def);
lv_obj_t *create_info_panel(lv_obj_t *parent);
void show_menu(lv_obj_t *menu);
void toggle_info_panel(lv_obj_t *panel);
void close_active_menu(void);

// Dialog functions
lv_obj_t *create_confirmation_dialog(const char *message, lv_event_cb_t confirm_cb);