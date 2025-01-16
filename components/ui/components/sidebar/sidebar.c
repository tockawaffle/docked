#include "sidebar.h"

lv_obj_t *separator(lv_obj_t *parent)
{
    lv_obj_t *separator = lv_obj_create(parent);

    lv_obj_remove_style_all(separator);
    lv_obj_set_size(separator, lv_pct(100), 1);
    lv_obj_set_style_bg_color(separator, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(separator, LV_OPA_COVER, 0);

    return separator;
}

lv_obj_t *logo_img(lv_obj_t *parent)
{
    lv_obj_t *image;
    image = lv_img_create(parent);
    
    // Set up image
    lv_img_set_src(image, "S:/assets/logos/logo.png");
    
    // Enable scaling
    lv_img_set_zoom(image, 256);  // 256 = no zoom (internal 8-bit fp format)
    lv_obj_set_size(image, 118, 118);
    lv_img_set_antialias(image, true);
    
    // Set alignment and position
    lv_obj_align(image, LV_ALIGN_TOP_MID, 0, 0);
    
    // Add a red background for debugging
    lv_obj_set_style_bg_color(image, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(image, LV_OPA_50, LV_PART_MAIN);

    return image;
}

lv_obj_t *create_sidebar_component(lv_obj_t *parent)
{
    // Sidebar
    lv_obj_t *sidebar = lv_obj_create(parent);

    // Started styling
    {
        lv_obj_remove_style_all(sidebar);
        lv_obj_set_size(sidebar, 118, 480);
        lv_obj_set_style_bg_color(sidebar, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, 0);
    }

    // Set up flex layout for the sidebar - change to row instead of column
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sidebar, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Add logo
    logo_img(sidebar);
    // Separator
    separator(sidebar);

    return sidebar;
}