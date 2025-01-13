#include "ui.h"
#include "colors.h"
#include "logos/logo.h"

#include "sidebar/sidebar.h"

static lv_obj_t *main_screen;
static lv_obj_t *perfmon;

void create_ui(void)
{
    main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);

    create_sidebar_component(main_screen);
}