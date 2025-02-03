// 29/01/2025 (DD/MM/YYYY for _those_ guys) - me cavemen, me forgot we in 2025
// I started balding while doing this. Guess I'm now a true programmer.

#include "menu_internals.h"

static lv_obj_t *active_menu = NULL;
static char wifi_name_buf[50];
static char signal_str_buf[50];

lv_obj_t *create_slide_menu(lv_obj_t *parent, const menu_def_t *menu_def)
{
    if (!parent || !menu_def)
    {
        return NULL;
    }

    lv_obj_t *menu = lv_obj_create(parent);
    if (!menu)
    {
        return NULL;
    }

    // Basic container setup
    lv_obj_remove_style_all(menu);
    lv_obj_set_size(menu, MENU_WIDTH, LV_SIZE_CONTENT);
    lv_obj_add_flag(menu, LV_OBJ_FLAG_HIDDEN);

    lv_obj_set_style_bg_color(menu, lv_color_hex(COLOR_SECONDARY), 0);
    lv_obj_set_style_bg_opa(menu, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(menu, SPACING_UNIT, 0);
    lv_obj_set_flex_flow(menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_border_side(menu, LV_BORDER_SIDE_FULL, LV_PART_MAIN);
    lv_obj_set_style_border_color(menu, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_border_opa(menu, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(menu, 2, 0);
    lv_obj_set_style_radius(menu, 15, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(menu);
    if (title)
    {
        lv_label_set_text(title, menu_def->title);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(title, lv_color_hex(COLOR_FOREGROUND), 0);
        lv_obj_set_style_pad_bottom(title, SPACING_UNIT * 2, 0);
    }

    for (size_t i = 0; i < menu_def->item_count; i++)
    {
        lv_obj_t *btn = lv_btn_create(menu);
        if (!btn)
        {
            continue;
        }

        lv_obj_set_size(btn, LV_PCT(90), LV_SIZE_CONTENT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_PRIMARY), 0);
        lv_obj_set_style_bg_opa(btn, LV_OPA_20, 0);
        lv_obj_set_style_radius(btn, BORDER_RADIUS, 0);
        lv_obj_set_style_pad_all(btn, SPACING_UNIT, 0);

        lv_obj_set_style_bg_opa(btn, LV_OPA_40, LV_STATE_PRESSED);

        lv_obj_t *label = lv_label_create(btn);
        if (label)
        {
            lv_label_set_text(label, menu_def->items[i].text);
            lv_obj_set_style_text_color(label, lv_color_hex(COLOR_FOREGROUND), 0);
            lv_obj_center(label);
        }

        if (menu_def->items[i].callback)
        {
            lv_obj_add_event_cb(btn, menu_def->items[i].callback, LV_EVENT_CLICKED, NULL);
        }

        lv_obj_set_style_margin_bottom(btn, SPACING_UNIT, 0);
    }

    return menu;
}

// Create a function specifically for info panels
lv_obj_t *create_info_panel(lv_obj_t *parent)
{
    if (!parent)
        return NULL;

    // Create base container
    lv_obj_t *panel = lv_obj_create(parent);
    if (!panel)
        return NULL;

    lv_obj_remove_style_all(panel);
    lv_obj_set_size(panel, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_pos(panel, SIDEBAR_WIDTH + SPACING_UNIT, SPACING_UNIT); // Position next to sidebar
    lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);

    // Panel styling
    lv_obj_set_style_bg_color(panel, lv_color_hex(COLOR_BACKGROUND), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(panel, SPACING_UNIT * 2, 0);
    lv_obj_set_style_radius(panel, BORDER_RADIUS, 0);
    lv_obj_set_style_border_width(panel, 2, 0);
    lv_obj_set_style_border_color(panel, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);

    // Update info text
    snprintf(wifi_name_buf, sizeof(wifi_name_buf), "Network: %s", ctx.wifi.name);
    snprintf(signal_str_buf, sizeof(signal_str_buf), "Signal: %d dBm", ctx.wifi.signal_strength);

    // Create each text line
    lv_obj_t *title = lv_label_create(panel);
    lv_label_set_text(title, "WiFi Details");
    lv_obj_set_style_text_color(title, lv_color_hex(COLOR_FOREGROUND), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_pad_bottom(title, SPACING_UNIT, 0);

    lv_obj_t *network = lv_label_create(panel);
    lv_label_set_text(network, wifi_name_buf);
    lv_obj_set_style_text_color(network, lv_color_hex(COLOR_FOREGROUND), 0);

    lv_obj_t *signal = lv_label_create(panel);
    lv_label_set_text(signal, signal_str_buf);
    lv_obj_set_style_text_color(signal, lv_color_hex(COLOR_FOREGROUND), 0);

    return panel;
}

void toggle_info_panel(lv_obj_t *panel)
{
    if (!panel)
        return;

    if (lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN))
    {
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
    }
}

void show_menu(lv_obj_t *menu)
{
    if (!menu)
        return;

    // If this menu is currently visible, hide it
    if (menu == active_menu && !lv_obj_has_flag(menu, LV_OBJ_FLAG_HIDDEN))
    {
        lv_obj_add_flag(menu, LV_OBJ_FLAG_HIDDEN);
        active_menu = NULL;
        return;
    }

    // If there's another menu visible, hide it
    if (active_menu && active_menu != menu)
    {
        lv_obj_add_flag(active_menu, LV_OBJ_FLAG_HIDDEN);
    }

    // Show this menu
    lv_obj_clear_flag(menu, LV_OBJ_FLAG_HIDDEN);
    active_menu = menu;

    // Slide animation
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, menu);
    lv_anim_set_time(&a, MENU_ANIM_TIME);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_values(&a, -MENU_WIDTH, 0);
    lv_anim_start(&a);
}
void close_active_menu(void)
{
    if (!active_menu)
        return;

    lv_obj_add_flag(active_menu, LV_OBJ_FLAG_HIDDEN);
    active_menu = NULL;
}

static void menu_cb(lv_event_t *e)
{
    lv_obj_t *dialog = lv_event_get_user_data(e);
    lv_obj_del(dialog);
}

lv_obj_t *create_confirmation_dialog(const char *message, lv_event_cb_t confirm_cb)
{
    lv_obj_t *dialog = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(dialog);
    lv_obj_set_size(dialog, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_center(dialog);
    lv_obj_set_style_bg_color(dialog, lv_color_hex(COLOR_BACKGROUND), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(dialog, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(dialog, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(dialog, 20, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(dialog, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_pad_all(dialog, 20, LV_PART_MAIN);

    lv_obj_t *content = lv_obj_create(dialog);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(content, 20, LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(content);
    lv_label_set_text(label, message);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t *btn_container = lv_obj_create(content);
    lv_obj_remove_style_all(btn_container);
    lv_obj_set_size(btn_container, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(btn_container, 10, LV_PART_MAIN);

    lv_obj_t *confirm_btn = lv_btn_create(btn_container);
    lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0xFF0000), LV_PART_MAIN);
    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "Confirm");
    if (confirm_cb)
    {
        lv_obj_add_event_cb(confirm_btn, confirm_cb, LV_EVENT_CLICKED, dialog);
    }

    lv_obj_t *cancel_btn = lv_btn_create(btn_container);
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "Cancel");
    lv_obj_add_event_cb(cancel_btn, menu_cb, LV_EVENT_CLICKED, dialog);

    return dialog;
}