#pragma once

#include "main_screen.h"
#include "main_types.h"
#include "colors.h"
#include "sd_card.h"
#include "lv_fs.h"
#include "wifi.h"
#include "wifi_config.h"
#include "logo.h"
#include "icons.h"

#define MAIN_TAG "MainScreen"

/**
 * @brief Creates a sidebar component
 * Sidebar contains the logo and some other functionalisties (TBD)
 */
lv_obj_t *create_sidebar(lv_obj_t *main_screen);