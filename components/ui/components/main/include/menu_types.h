#pragma once
#ifndef MENU_TYPES_H
#define MENU_TYPES_H

#include <lvgl.h>

#define MENU_WIDTH 240
#define MENU_ANIM_TIME 200

typedef struct menu_item {
    const char *text;
    lv_event_cb_t callback;
} menu_item_t;

typedef struct menu_def {
    const char *title;
    const menu_item_t *items;
    size_t item_count;
} menu_def_t;

#endif // MENU_TYPES_H