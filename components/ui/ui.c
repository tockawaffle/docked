#include "ui.h"
#include "colors.h"
#include "logos/logo.h"

#include "sidebar/sidebar.h"
#include "splash/splash.h"

static lv_obj_t *main_screen;

void create_ui(void)
{
    main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);

    splash_screen(main_screen);
}