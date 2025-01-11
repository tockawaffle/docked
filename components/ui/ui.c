#include "ui.h"
#include "colors.h"
#include "logos/logo.h"

#include "sidebar/sidebar.h"
#include "sd_card/sd_card.h"

static lv_obj_t *main_screen;
static lv_obj_t *perfmon;

void create_ui(void)
{
    if (waveshare_sd_card_init() == ESP_OK)
    {
        // Test SD card functionality
        waveshare_sd_card_test();
    }

    // Create main screen
    main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);

    create_sidebar_component(main_screen);
}