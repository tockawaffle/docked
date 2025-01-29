#include "main_internals.h"
#include "menu_internals.h"

#define SIDEBAR_TAG "Sidebar"

static void wifi_clicked_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_PRESSED)
        return;
    // TODO: Implement WiFi toggle
}
static void power_off_ev(lv_event_t *e)
{
    lv_obj_t *dialog = lv_event_get_user_data(e);
    lv_obj_del(dialog);
    // TODO: Implement power off
    ESP_LOGI(SIDEBAR_TAG, "Power off confirmed");
}

static void power_clicked_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_PRESSED)
        return;

    create_confirmation_dialog("Are you sure you want to power off?", power_off_ev);
}

static void create_bottom_buttons(lv_obj_t *sidebar)
{
    // Create container for bottom buttons with flex
    lv_obj_t *bottom_container = lv_obj_create(sidebar);
    lv_obj_remove_style_all(bottom_container);
    lv_obj_set_size(bottom_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(bottom_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_container, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(bottom_container, 10, LV_PART_MAIN);

    // Push container to bottom using flex grow
    lv_obj_set_flex_grow(bottom_container, 1);
    lv_obj_set_style_margin_top(bottom_container, LV_PCT(100), LV_PART_MAIN);

    // WiFi button
    ctx.wifi_indicator = lv_btn_create(bottom_container);
    lv_obj_remove_style_all(ctx.wifi_indicator);
    lv_obj_set_size(ctx.wifi_indicator, 40, 40);
    lv_obj_set_style_radius(ctx.wifi_indicator, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ctx.wifi_indicator, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ctx.wifi_indicator, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(ctx.wifi_indicator, wifi_clicked_cb, LV_EVENT_PRESSED, NULL);

    lv_obj_t *wifi_label = lv_label_create(ctx.wifi_indicator);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_obj_center(wifi_label);
    lv_obj_set_style_text_color(wifi_label,
                                ctx.wifi_state == WIFI_CONNECTED ? lv_color_hex(0x00FF00) : lv_color_hex(0xFF0000),
                                LV_PART_MAIN);

    // Power button
    lv_obj_t *power_btn = lv_btn_create(bottom_container);
    lv_obj_remove_style_all(power_btn);
    lv_obj_set_size(power_btn, 40, 40);
    lv_obj_set_style_radius(power_btn, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(power_btn, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(power_btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_add_event_cb(power_btn, power_clicked_cb, LV_EVENT_PRESSED, NULL);

    lv_obj_t *power_label = lv_label_create(power_btn);
    lv_label_set_text(power_label, LV_SYMBOL_POWER);
    lv_obj_center(power_label);
    lv_obj_set_style_text_color(power_label, lv_color_hex(0xFF0000), LV_PART_MAIN);
}

void home_clicked_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_PRESSED)
        return;

    ESP_LOGI(SIDEBAR_TAG, "Home clicked");

    static lv_obj_t *menu = NULL;
    if (!menu)
    {
        ESP_LOGI(SIDEBAR_TAG, "Home menu sent to be created");
        menu = create_slide_menu(menu_ctx.menu_cont, &home_menu);
        show_menu(menu);
        return;
    }
    show_menu(menu);
}

void settings_clicked_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_PRESSED)
        return;

    ESP_LOGI(SIDEBAR_TAG, "Settings clicked");

    static lv_obj_t *menu = NULL;
    if (!menu)
    {
        menu = create_slide_menu(menu_ctx.menu_cont, &settings_menu);
    }
    show_menu(menu);
}

static uint32_t calculate_checksum(const void *data, size_t length)
{
    const uint8_t *ptr = (const uint8_t *)data;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < length; i++)
    {
        crc = (crc >> 8) ^ crc32_table[(crc ^ ptr[i]) & 0xFF];
    }

    return crc ^ 0xFFFFFFFF;
}

static bool validate_button_data(const sidebar_btn_t *btn)
{
    if (!btn || !btn->text)
        return false;

    // Validate text length
    size_t text_len = strnlen(btn->text, MAX_BUTTON_TEXT_LENGTH + 1);
    if (!IS_VALID_TEXT_LENGTH(text_len))
        return false;

    // Validate icon type
    if (!IS_VALID_ICON_TYPE(btn->icon_type))
        return false;

    // Validate icon data
    if (btn->icon_type == SIDEBAR_ICON_SYMBOL && !btn->icon.symbol)
        return false;
    if (btn->icon_type == SIDEBAR_ICON_IMAGE && !btn->icon.image)
        return false;

    // Verify checksum
    uint32_t calculated_checksum = calculate_checksum(btn, sizeof(sidebar_btn_t) - sizeof(uint32_t));
    if (calculated_checksum != btn->checksum)
        return false;

    return true;
}

static void update_wifi_indicator(void)
{
    if (!ctx.wifi_indicator)
        return;

    const char *wifi_symbol = (ctx.wifi_state == WIFI_CONNECTED) ? LV_SYMBOL_WIFI : LV_SYMBOL_CLOSE;
    lv_obj_t *label = lv_obj_get_child(ctx.wifi_indicator, 0);
    if (label)
    {
        lv_label_set_text(label, wifi_symbol);
    }
}

lv_obj_t *create_sidebar_button(lv_obj_t *parent, sidebar_btn_t *btn_data)
{
    if (!parent || ctx.button_count >= MAX_BUTTONS ||
        !validate_button_data(btn_data))
    {
        return NULL;
    }

    lv_obj_t *btn = lv_btn_create(parent);
    if (!btn)
        return NULL;

    // Setup button styles
    lv_obj_remove_style_all(btn);
    lv_obj_set_size(btn, SIDEBAR_BUTTON_WIDTH, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn, 10, LV_PART_MAIN);

    lv_obj_t *cont = lv_obj_create(btn);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(cont, 10, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_CLICKABLE);

    // Create icon
    if (btn_data->icon_type == SIDEBAR_ICON_IMAGE)
    {
        lv_obj_t *icon = lv_img_create(cont);
        lv_img_set_src(icon, btn_data->icon.image);
        lv_obj_set_size(icon, ICON_SIZE, ICON_SIZE);
        lv_obj_clear_flag(icon, LV_OBJ_FLAG_CLICKABLE);
    }
    else
    {
        lv_obj_t *symbol = lv_label_create(cont);
        lv_label_set_text(symbol, btn_data->icon.symbol);
        lv_obj_set_style_text_font(symbol, &lv_font_montserrat_20, 0);
        lv_obj_clear_flag(symbol, LV_OBJ_FLAG_CLICKABLE);
    }

    lv_obj_t *label = lv_label_create(cont);
    lv_label_set_text(label, btn_data->text);
    lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);

    if (btn_data->callback)
    {
        lv_obj_add_event_cb(btn, btn_data->callback, LV_EVENT_PRESSED, NULL);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_STATE_PRESSED);
        lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_SECONDARY), LV_STATE_PRESSED);
    }

    ctx.button_count++;
    return btn;
}

lv_obj_t *create_menu_ctx(lv_obj_t *main_cont)
{
    menu_ctx.menu_cont = lv_obj_create(main_cont);
    if (!menu_ctx.menu_cont)
    {
        ESP_LOGE(SIDEBAR_TAG, "Failed to create menu container");
        return NULL;
    }

    lv_obj_remove_style_all(menu_ctx.menu_cont);
    lv_obj_set_size(menu_ctx.menu_cont, LV_SIZE_CONTENT, LV_PCT(100));
    lv_obj_set_style_pad_left(menu_ctx.menu_cont, 0, LV_PART_MAIN);
    lv_obj_set_flex_flow(menu_ctx.menu_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(menu_ctx.menu_cont,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);

    // Position it right after the sidebar
    lv_obj_set_pos(menu_ctx.menu_cont, SIDEBAR_WIDTH, 0);

    return menu_ctx.menu_cont;
}

lv_obj_t *create_sidebar(lv_obj_t *main_screen)
{
    if (!main_screen)
        return NULL;

    lv_obj_t *main_cont = lv_obj_create(main_screen);
    lv_obj_remove_style_all(main_cont);
    lv_obj_set_size(main_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(main_cont, 0, 0);

    // Create and configure sidebar container
    lv_obj_t *sidebar = lv_obj_create(main_cont);
    if (!sidebar)
        return NULL;

    lv_obj_remove_style_all(sidebar);
    lv_obj_set_size(sidebar, SIDEBAR_WIDTH, LV_PCT(100));
    lv_obj_set_style_bg_color(sidebar, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(sidebar, LV_OPA_COVER, LV_PART_MAIN);

    // Create top section container
    lv_obj_t *top_section = lv_obj_create(sidebar);
    lv_obj_remove_style_all(top_section);
    lv_obj_set_size(top_section, LV_PCT(100), LV_PCT(85)); // Use 85% of height for top section
    lv_obj_set_flex_flow(top_section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(top_section, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(top_section, 0, LV_PART_MAIN);

    // Create logo section in top section
    lv_obj_t *logo_container = lv_obj_create(top_section);
    lv_obj_remove_style_all(logo_container);
    lv_obj_set_size(logo_container, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_top(logo_container, 20, LV_PART_MAIN);

    lv_obj_t *logo_img = lv_img_create(logo_container);
    lv_img_set_src(logo_img, &logo);
    lv_obj_set_size(logo_img, LOGO_SIZE, LOGO_SIZE);
    lv_obj_center(logo_img);
    lv_img_set_antialias(logo_img, true);

    // Add separator
    lv_obj_t *sep = separator(top_section, 80, 20, 10);

    // Initialize navigation buttons
    sidebar_btn_t buttons[] = {
        {.text = "Home",
         .text_length = 4,
         .icon_type = SIDEBAR_ICON_SYMBOL,
         .icon.symbol = LV_SYMBOL_HOME,
         .callback = home_clicked_cb,
         .checksum = 0},
        {.text = "Settings",
         .text_length = 8,
         .icon_type = SIDEBAR_ICON_SYMBOL,
         .icon.symbol = LV_SYMBOL_SETTINGS,
         .callback = settings_clicked_cb,
         .checksum = 0}};

    // Calculate checksums
    for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++)
    {
        buttons[i].checksum = calculate_checksum(&buttons[i], sizeof(sidebar_btn_t) - sizeof(uint32_t));
    }

    // Create navigation buttons in top section
    for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++)
    {
        create_sidebar_button(top_section, &buttons[i]);
    }

    // Create bottom section container
    lv_obj_t *bottom_section = lv_obj_create(sidebar);
    lv_obj_remove_style_all(bottom_section);
    lv_obj_set_size(bottom_section, LV_PCT(100), LV_PCT(15)); // Use 15% of height for bottom section
    lv_obj_set_style_pad_all(bottom_section, 10, LV_PART_MAIN);
    lv_obj_align(bottom_section, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Create bottom buttons in bottom section
    create_bottom_buttons(bottom_section);

    // Create menu context
    create_menu_ctx(main_cont);

    return sidebar;
}