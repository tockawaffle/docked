#include "lvgl.h"
#include "colors.h"

#include "./components/sidebar.h"

static lv_obj_t *main_screen;

// Creates and handles the main UI.
void create_ui(void)
{
    // Create main screen
    main_screen = lv_obj_create(NULL);

    // Set style of the screen
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_SECONDARY), 0);
    lv_obj_set_style_opa(main_screen, 255, 0);
    lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);

    sidebar_t *sidebar = create_sidebar(main_screen);
    if (!sidebar) {
        return;
    }

    lv_scr_load(main_screen);
}
