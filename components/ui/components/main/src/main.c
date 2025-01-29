#include "main_internals.h"

lv_obj_t *separator(lv_obj_t *parent, uint8_t width_pct, lv_coord_t margin_top, lv_coord_t margin_bottom)
{
    if (!parent)
        return NULL;

    lv_obj_t *separator = lv_obj_create(parent);
    if (!separator)
        return NULL;

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

static sidebar_ctx_t *g_sidebar_ctx = NULL; // Global context for cleanup

void main_screen_init(void)
{
    // Create main screen
    lv_obj_t *main_screen = lv_obj_create(NULL);
    if (!main_screen)
        return;

    lv_scr_load(main_screen);
    lv_obj_set_style_bg_color(main_screen, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);

    // Create sidebar with initialized context
    lv_obj_t *sidebar = create_sidebar(main_screen);
    if (!sidebar)
    {
        // Handle sidebar creation failure
        ESP_LOGE(MAIN_TAG, "Could not initialize sidebar");
        lv_obj_del(main_screen);
        return;
    }

    // Register cleanup on screen delete if your system supports it
    // lv_obj_add_event_cb(main_screen, cleanup_main_screen, LV_EVENT_DELETE, NULL);
}