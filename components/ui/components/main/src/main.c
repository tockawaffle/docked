#include "main_internals.h"

lv_obj_t *separator(lv_obj_t *parent)
{
    lv_obj_t *separator = lv_obj_create(parent);
    lv_obj_remove_style_all(separator);

    lv_obj_set_size(separator, LV_PCT(100), 2); // Made slightly thicker for visibility
    lv_obj_set_style_bg_color(separator, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(separator, LV_OPA_COVER, LV_PART_MAIN);

    return separator;
}

void main_screen_init()
{
    lv_obj_t *main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);

    // Sidebar
    lv_obj_t *sidebar = lv_obj_create(main_screen);
    lv_obj_remove_style_all(sidebar);

    // Set sidebar properties
    lv_obj_set_size(sidebar, 120, LV_PCT(100));
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, LV_PART_MAIN); // Fixed part parameter

    // Change flex flow to start from top
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sidebar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    // Logo container to ensure proper placement
    lv_obj_t *logo_container = lv_obj_create(sidebar);
    lv_obj_remove_style_all(logo_container);
    lv_obj_set_size(logo_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_top(logo_container, 20, LV_PART_MAIN);

    // Logo
    lv_obj_t *logo_img = lv_img_create(logo_container);
    lv_img_set_src(logo_img, &logo);
    lv_obj_set_size(logo_img, 50, 50);
    lv_obj_center(logo_img);
    lv_img_set_antialias(logo_img, true);

    // Separator
    lv_obj_t *sep = separator(sidebar);
    // Set a padding from the logo image
    
}
