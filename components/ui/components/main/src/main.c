#include "main_internals.h"

void main_screen_init()
{
    lv_obj_t *main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);

    // Sidebar
    lv_obj_t *sidebar = lv_obj_create(main_screen);
    lv_obj_remove_style_all(sidebar);

    {
        lv_obj_set_size(sidebar, 120, LV_PCT(100));
        lv_obj_set_style_bg_color(sidebar, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, LV_PART_MAIN);

        lv_obj_t *logo_img = lv_img_create(sidebar);

        // Get from mount_point
        
        lv_img_set_src(logo_img, &logo);
        lv_obj_set_size(logo_img, 50, 50);
        lv_obj_center(logo_img);
    }
};