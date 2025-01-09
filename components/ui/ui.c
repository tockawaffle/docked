#include "ui.h"
#include "colors.h"

static lv_obj_t *main_screen;
static lv_obj_t *perfmon;


// Creates and handles the main UI.
void create_ui(void)
{
    // Create main screen
    main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);

    // Sidebar
    lv_obj_t *sidebar = lv_obj_create(main_screen);
    lv_obj_remove_style_all(sidebar);
    lv_obj_set_size(sidebar, 120, 480);
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN); // Sets the main color
    lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, 0);                             // Sets the bg color opacity
   

    // // Set up flex layout for the sidebar
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sidebar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

}