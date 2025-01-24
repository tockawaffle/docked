#include "main_internals.h"

lv_obj_t *separator(lv_obj_t *parent, uint8_t width_pct, lv_coord_t margin_top, lv_coord_t margin_bottom)
{
    lv_obj_t *separator = lv_obj_create(parent);
    lv_obj_remove_style_all(separator);

    width_pct = width_pct == 0 ? 100 : width_pct;

    lv_obj_set_size(separator, LV_PCT(width_pct), 2);
    lv_obj_center(separator);
    lv_obj_set_style_bg_color(separator, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(separator, LV_OPA_COVER, LV_PART_MAIN);

    if (margin_top > 0)
    {
        lv_obj_set_style_margin_top(separator, margin_top, LV_PART_MAIN);
    }
    if (margin_bottom > 0)
    {
        lv_obj_set_style_margin_bottom(separator, margin_bottom, LV_PART_MAIN);
    }

    return separator;
}

void main_screen_init()
{
    lv_obj_t *main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);

    // Sidebar
    create_sidebar(main_screen);
}
