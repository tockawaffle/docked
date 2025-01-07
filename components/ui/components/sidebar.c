#include "sidebar.h"

// Forward declarations of all helper functions
static void create_toggle_button(sidebar_t *sb);
static void create_logo_section(sidebar_t *sb);
static void create_button_sections(sidebar_t *sb);
static void create_button_group(lv_obj_t *parent, const sidebar_button_t *buttons,
                                size_t button_count, bool is_open);
static void sidebar_width_anim_cb(void *var, int32_t value);
static lv_obj_t *create_separator(lv_obj_t *parent);
static lv_obj_t *create_sidebar_button(lv_obj_t *parent, const sidebar_button_t *btn_data, bool is_open);

// Animation callback
static void sidebar_width_anim_cb(void *var, int32_t value) {
    lv_obj_t *sidebar_obj = (lv_obj_t *)var;
    lv_obj_set_width(sidebar_obj, value);

    // Get if we're closer to open or closed state
    bool is_more_open = value > (48 + (128 - 48) / 2);  // Using fixed values for clarity

    // Update button alignments
    uint32_t btn_count = lv_obj_get_child_cnt(sidebar_obj);
    for(uint32_t i = 0; i < btn_count; i++) {
        lv_obj_t *child = lv_obj_get_child(sidebar_obj, i);
        if (lv_obj_check_type(child, &lv_btn_class)) {
            // For each button, update its padding to center the icon
            int32_t padding = is_more_open ? 4 : (value - 24) / 2;
            if (padding < 0) padding = 0;
            lv_obj_set_style_pad_left(child, padding, 0);
            
            // Find and update the label if it exists
            uint32_t btn_child_count = lv_obj_get_child_cnt(child);
            for(uint32_t j = 0; j < btn_child_count; j++) {
                lv_obj_t *btn_child = lv_obj_get_child(child, j);
                if (lv_obj_check_type(btn_child, &lv_label_class) && j > 0) {
                    // This is a label (not the icon)
                    lv_obj_set_style_opa(btn_child, is_more_open ? LV_OPA_COVER : 0, 0);
                }
            }
        }
    }
}


// Toggle button callback
static void toggle_cb(lv_event_t *e) {
    sidebar_t *sb = (sidebar_t *)lv_event_get_user_data(e);
    
    // Create animation
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, sb->sidebar);  // Pass the sidebar object, not the structure
    lv_anim_set_values(&a, lv_obj_get_width(sb->sidebar),
                      sb->is_open ? sb->closed_width : sb->open_width);
    lv_anim_set_time(&a, 300);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&a, sidebar_width_anim_cb);
    
    lv_anim_start(&a);
    
    // Update toggle button icon
    lv_obj_t *icon = lv_obj_get_child(sb->toggle_btn, 0);
    lv_label_set_text(icon, sb->is_open ? LV_SYMBOL_RIGHT : LV_SYMBOL_LEFT);
    
    sb->is_open = !sb->is_open;
}

// Create the main separator
static lv_obj_t *create_separator(lv_obj_t *parent)
{
    lv_obj_t *sep = lv_obj_create(parent);
    lv_obj_remove_style_all(sep);
    lv_obj_set_size(sep, lv_pct(100), 1);
    lv_obj_set_style_bg_color(sep, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(sep, LV_OPA_COVER, 0);
    return sep;
}

static void create_toggle_button(sidebar_t *sb)
{
    sb->toggle_btn = lv_btn_create(sb->sidebar);
    lv_obj_remove_style_all(sb->toggle_btn);
    lv_obj_set_size(sb->toggle_btn, 32, 32);
    lv_obj_set_style_translate_x(sb->toggle_btn, 8, 0);
    lv_obj_set_style_translate_y(sb->toggle_btn, 12, 0);

    lv_obj_t *icon = lv_label_create(sb->toggle_btn);
    lv_label_set_text(icon, LV_SYMBOL_LEFT);
    lv_obj_center(icon);

    lv_obj_add_event_cb(sb->toggle_btn, toggle_cb, LV_EVENT_CLICKED, sb);
}

// Create the toggle button
static lv_obj_t* create_sidebar_button(lv_obj_t *parent, const sidebar_button_t *btn_data, bool is_open) {
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, lv_pct(100), 32);
    
    // Set initial padding based on sidebar state
    lv_obj_set_style_pad_left(btn, 
        is_open ? 4 : ((48 - 24) / 2),  // When collapsed, center the 24px icon in 48px width
        0);
    
    // Create icon
    if (strcmp(btn_data->icon, "●") == 0) {
        lv_obj_t *dot = lv_obj_create(btn);
        lv_obj_remove_style_all(dot);
        lv_obj_set_size(dot, 8, 8);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(dot, btn_data->icon_color, 0);
        lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    } else {
        lv_obj_t *icon = lv_label_create(btn);
        lv_label_set_text(icon, btn_data->icon);
    }
    
    // Create label with initial visibility
    if (btn_data->label != NULL) {
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, btn_data->label);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_pad_left(label, 12, 0);
        lv_obj_set_style_opa(label, is_open ? LV_OPA_COVER : 0, 0);
    }
    
    if (btn_data->callback) {
        lv_obj_add_event_cb(btn, btn_data->callback, LV_EVENT_CLICKED, NULL);
    }
    
    return btn;
}

static void create_logo_section(sidebar_t *sb)
{
    // Create logo container with flex layout
    lv_obj_t *logo_cont = lv_obj_create(sb->sidebar);
    lv_obj_remove_style_all(logo_cont);
    lv_obj_set_size(logo_cont, lv_pct(100), 40);
    lv_obj_set_style_pad_top(logo_cont, 64, 0); // mt-16 equivalent
    lv_obj_set_flex_flow(logo_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(logo_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create image container to maintain aspect ratio
    lv_obj_t *img_cont = lv_obj_create(logo_cont);
    lv_obj_remove_style_all(img_cont);
    lv_obj_set_size(img_cont, 40, 40);

    // Create the image object
    lv_obj_t *logo = lv_img_create(img_cont);

    // Set up image properties
    lv_img_set_src(logo, "/home/tocka/ESP32-UI/components/ui/logos/logo.png"); // Adjust path according to your filesystem

    // Optional: Add error handling for image loading
    if (lv_img_get_src(logo) == LV_IMG_SRC_UNKNOWN)
    {
        // Image failed to load, you might want to show a placeholder
        lv_obj_t *fallback = lv_label_create(img_cont);
        lv_label_set_text(fallback, LV_SYMBOL_IMAGE);
        lv_obj_center(fallback);
        lv_obj_del(logo);
    }
    else
    {
        // Image loaded successfully, center it
        lv_obj_center(logo);

        // Optional: Add image processing styles
        lv_obj_set_style_img_recolor(logo, lv_color_white(), 0); // Make it white
        lv_obj_set_style_img_recolor_opa(logo, 255, 0);          // Full recolor opacity
    }
}
// Create all button sections
static void create_button_sections(sidebar_t *sb)
{
    const sidebar_button_t settings_buttons[] = {
        {.icon = LV_SYMBOL_SETTINGS, .label = "Settings"},
        {.icon = LV_SYMBOL_AUDIO, .label = "Screen"},
        {.icon = "●", .label = "Connected", .icon_color = lv_color_hex(0x00FF00)}};

    create_button_group(sb->buttons_container, settings_buttons,
                        sizeof(settings_buttons) / sizeof(settings_buttons[0]), sb->is_open);
    create_separator(sb->buttons_container);

    const sidebar_button_t control_buttons[] = {
        {.icon = LV_SYMBOL_KEYBOARD, .label = "Macro"},
        {.icon = LV_SYMBOL_VOLUME_MAX, .label = "Audio"},
        {.icon = LV_SYMBOL_LIST, .label = "Scenes"}};

    create_button_group(sb->buttons_container, control_buttons,
                        sizeof(control_buttons) / sizeof(control_buttons[0]), sb->is_open);
    create_separator(sb->buttons_container);

    const sidebar_button_t network_buttons[] = {
        {.icon = LV_SYMBOL_WIFI, .label = "Wi-Fi"}};

    create_button_group(sb->buttons_container, network_buttons,
                        sizeof(network_buttons) / sizeof(network_buttons[0]), sb->is_open);
}

// Create a group of buttons
static void create_button_group(lv_obj_t *parent, const sidebar_button_t *buttons,
                                size_t button_count, bool is_open)
{
    lv_obj_t *group = lv_obj_create(parent);
    lv_obj_remove_style_all(group);
    lv_obj_set_size(group, lv_pct(200), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_hor(group, 4, 0);
    lv_obj_set_flex_flow(group, LV_FLEX_FLOW_COLUMN);

    for (size_t i = 0; i < button_count; i++)
    {
        create_sidebar_button(group, &buttons[i], is_open);
    }
}

// Main creation function
sidebar_t *create_sidebar(lv_obj_t *parent)
{
    sidebar_t *sb = lv_mem_alloc(sizeof(sidebar_t));
    if (sb == NULL)
        return NULL;

    sb->is_open = true;
    sb->open_width = 128;
    sb->closed_width = 48;

    // Create main container
    sb->sidebar = lv_obj_create(parent);
    lv_obj_remove_style_all(sb->sidebar);
    lv_obj_set_size(sb->sidebar, sb->open_width, 480);

    // Set up styling
    lv_obj_set_style_bg_color(sb->sidebar, lv_color_hex(0x1e1e2e), 0);
    lv_obj_set_style_bg_opa(sb->sidebar, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(sb->sidebar, lv_color_white(), 0);

    // Set up flex layout
    lv_obj_set_flex_flow(sb->sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sb->sidebar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    create_toggle_button(sb);
    create_logo_section(sb);

    // Create button container
    sb->buttons_container = lv_obj_create(sb->sidebar);
    lv_obj_remove_style_all(sb->buttons_container);
    lv_obj_set_size(sb->buttons_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(sb->buttons_container, LV_FLEX_FLOW_COLUMN);

    create_button_sections(sb);

    return sb;
}