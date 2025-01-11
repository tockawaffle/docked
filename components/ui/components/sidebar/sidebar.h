#include <lvgl.h>
#include <esp_log.h>
#include <colors.h>

#ifndef SIDEBAR_H
#define SIDEBAR_H

static const char *TAG = "SIDEBAR";

lv_obj_t *create_sidebar_component(lv_obj_t *parent);

#endif