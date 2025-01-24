#include "main_internals.h"

lv_obj_t *create_sidebar_button(lv_obj_t *parent, sidebar_btn_t *btn_data)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, LV_PCT(80), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn, 10, LV_PART_MAIN);

    lv_obj_t *cont = lv_obj_create(btn);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont, 10, 0);
    lv_obj_set_style_margin_top(cont, 10, LV_PART_MAIN);
    

    if (btn_data->icon_type == SIDEBAR_ICON_IMAGE)
    {
        lv_obj_t *icon = lv_img_create(cont);
        lv_img_set_src(icon, btn_data->icon.image);
        lv_obj_set_size(icon, 48, 48);
    }
    else
    {
        lv_obj_t *symbol = lv_label_create(cont);
        lv_label_set_text(symbol, btn_data->icon.symbol);
        lv_obj_set_style_text_font(symbol, &lv_font_montserrat_20, 0);
    }

    lv_obj_t *label = lv_label_create(cont);
    lv_label_set_text(label, btn_data->text);

    if (btn_data->callback)
    {
        lv_obj_add_event_cb(btn, btn_data->callback, LV_EVENT_CLICKED, NULL);
    }

    return btn;
}

void home_clicked_cb(lv_event_t *ev)
{
}

void settings_clicked_cb(lv_event_t *e) {}

lv_obj_t *create_sidebar(lv_obj_t *main_screen)
{
    lv_obj_t *sidebar = lv_obj_create(main_screen);
    lv_obj_remove_style_all(sidebar);

    // Set sidebar properties
    lv_obj_set_size(sidebar, 145, LV_PCT(100));
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, LV_PART_MAIN); // Fixed part parameter

    // Change flex flow to start from top
    lv_obj_set_flex_flow(sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sidebar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

    lv_obj_t *logo_container = lv_obj_create(sidebar);             // Logo container to ensure proper placement
    lv_obj_remove_style_all(logo_container);                       // Removes all styles to default styles to be applied
    lv_obj_set_size(logo_container, LV_PCT(100), LV_SIZE_CONTENT); // Sets the size of the container
    lv_obj_set_style_pad_top(logo_container, 20, LV_PART_MAIN);    // Sets the padding to the container itself

    lv_obj_t *logo_img = lv_img_create(logo_container); // Creates logo image
    lv_img_set_src(logo_img, &logo);                    // Sets its source
    lv_obj_set_size(logo_img, 50, 50);                  // Sets its size
    lv_obj_center(logo_img);                            // Center it to the component
    lv_img_set_antialias(logo_img, true);               // Configure antialiasing

    lv_obj_t *sep = separator(sidebar, 80, 20, 10); // Separator

    sidebar_btn_t buttons[] = {
        {"Home",
         .icon_type = SIDEBAR_ICON_SYMBOL,
         .icon.symbol = LV_SYMBOL_HOME,
         .callback = home_clicked_cb},
        {.text = "Settings",
         .icon_type = SIDEBAR_ICON_SYMBOL,
         .icon.symbol = LV_SYMBOL_SETTINGS,
         .callback = settings_clicked_cb}};

    for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++)
    {
        create_sidebar_button(sidebar, &buttons[i]);
    }

    return sidebar;
}
