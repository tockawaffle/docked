#pragma once

#include <lvgl.h>

typedef enum
{
    SIDEBAR_ICON_SYMBOL,
    SIDEBAR_ICON_IMAGE
} sidebar_icon_type_t;

typedef struct
{
    const char *text;
    sidebar_icon_type_t icon_type;
    union
    {
        const char *symbol;
        const lv_img_dsc_t *image;
    } icon;
    lv_event_cb_t callback;
} sidebar_btn_t;