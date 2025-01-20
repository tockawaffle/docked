#include "splash_internal.h"

void splash_cleanup_wifi_popup(splash_ctx_t *ctx);
void splash_cleanup_wifi_list(splash_ctx_t *ctx);

void splash_wifi_create_network_list(splash_ctx_t *ctx, wifi_scan_result_t *scan_result)
{
    // Create scrollable container if it doesn't exist
    if (ctx->networks_list == NULL)
    {
        ctx->networks_list = lv_obj_create(lv_scr_act());
        lv_obj_set_size(ctx->networks_list, LV_PCT(90), 200);
        lv_obj_align_to(ctx->networks_list, ctx->debug_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
        lv_obj_set_flex_flow(ctx->networks_list, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(ctx->networks_list, 10, 0);
        lv_obj_set_style_bg_color(ctx->networks_list, lv_color_hex(0x202020), 0);
        lv_obj_set_style_bg_opa(ctx->networks_list, LV_OPA_90, 0);
        lv_obj_set_style_border_width(ctx->networks_list, 2, 0);
        lv_obj_set_style_border_color(ctx->networks_list, lv_color_hex(COLOR_PRIMARY), 0);
        lv_obj_set_style_pad_all(ctx->networks_list, 10, 0);
        lv_obj_set_style_radius(ctx->networks_list, 10, 0);
        lv_obj_set_scrollbar_mode(ctx->networks_list, LV_SCROLLBAR_MODE_ACTIVE);

        // Add title
        lv_obj_t *title = lv_label_create(ctx->networks_list);
        lv_label_set_text(title, "Available Networks");
        lv_obj_set_style_text_color(title, lv_color_hex(COLOR_PRIMARY), 0);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
        lv_obj_set_style_pad_bottom(title, 10, 0);
    }

    // Create network buttons
    for (int i = 0; i < scan_result->ap_count; i++)
    {
        // Create button container
        lv_obj_t *btn = lv_btn_create(ctx->networks_list);
        lv_obj_set_size(btn, LV_PCT(100), 50);
        lv_obj_set_style_radius(btn, 5, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x303030), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_PRIMARY), LV_STATE_PRESSED);
        lv_obj_set_style_shadow_width(btn, 10, 0);
        lv_obj_set_style_shadow_spread(btn, 1, 0);
        lv_obj_set_style_shadow_color(btn, lv_color_hex(0x000000), 0);
        lv_obj_set_style_pad_all(btn, 15, 0); // Add padding to the button itself

        // Create a flex container directly in the button
        lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // WiFi icon
        lv_obj_t *wifi_icon = lv_label_create(btn);
        lv_label_set_text(wifi_icon, LV_SYMBOL_WIFI);
        lv_obj_set_style_text_color(wifi_icon, lv_color_hex(COLOR_PRIMARY), 0);
        lv_obj_set_style_pad_right(wifi_icon, 10, 0); // Add spacing after icon

        char *ssid_copy = malloc(strlen(scan_result->ssids[i]) + 1);
        strcpy(ssid_copy, scan_result->ssids[i]);

        ESP_LOGI(SPLASH_TAG, "Creating network button for SSID: '%s' (len: %d)",
                 ssid_copy, strlen(ssid_copy));

        // SSID label
        lv_obj_t *ssid_label = lv_label_create(btn);
        lv_label_set_text(ssid_label, scan_result->ssids[i]);
        lv_obj_set_flex_grow(ssid_label, 1); // Let SSID label take available space

        // Signal strength indicator
        char rssi_str[32];
        int signal_strength = (scan_result->rssis[i] + 100) * 2;
        if (signal_strength > 100)
            signal_strength = 100;
        if (signal_strength < 0)
            signal_strength = 0;
        snprintf(rssi_str, sizeof(rssi_str), "%d%%", signal_strength);

        lv_obj_t *rssi_label = lv_label_create(btn);
        lv_label_set_text(rssi_label, rssi_str);
        lv_obj_set_style_text_color(rssi_label,
                                    signal_strength > 70 ? lv_color_hex(0x00FF00) : signal_strength > 40 ? lv_color_hex(0xFFFF00)
                                                                                                         : lv_color_hex(0xFF0000),
                                    0);
        lv_obj_set_style_pad_left(rssi_label, 10, 0); // Add spacing before RSSI

        lv_obj_set_user_data(btn, ssid_copy);
        lv_obj_add_event_cb(btn, splash_on_wifi_button_click, LV_EVENT_CLICKED, ssid_copy);

        // Make sure the button captures all events
        lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }
}

void splash_on_wifi_button_click(lv_event_t *e)
{
    splash_ctx_t *ctx = splash_screen_get_context();
    const char *ssid = lv_event_get_user_data(e);

    ESP_LOGI(SPLASH_TAG, "Current Context: %p", ctx);
    ESP_LOGI(SPLASH_TAG, "Selected WiFi SSID (A): %s", ssid);

    // Store the selected SSID
    strncpy(ctx->selected_ssid, ssid, sizeof(ctx->selected_ssid) - 1);
    ctx->selected_ssid[sizeof(ctx->selected_ssid) - 1] = '\0';

    ESP_LOGI(SPLASH_TAG, "Selected WiFi SSID (B): %s", ctx->selected_ssid);

    // Update the SSID label in the popup
    lv_obj_t *ssid_label = lv_obj_get_child(ctx->password_popup, 2);
    if (ssid_label)
    {
        lv_label_set_text_fmt(ssid_label, "Network: %s", ctx->selected_ssid);
    }

    // Clear any previous password
    lv_textarea_set_text(ctx->password_input, "");

    // Show popup and focus password input
    lv_obj_clear_flag(ctx->password_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_state(ctx->password_input, LV_STATE_FOCUSED);
}

void splash_on_password_submit(lv_event_t *e)
{
    splash_ctx_t *ctx = splash_screen_get_context();
    const char *password = lv_textarea_get_text(ctx->password_input);

    ESP_LOGI(SPLASH_TAG, "Starting password submit - Current state: %d", ctx->state);

    // Store credentials locally before cleanup
    wifi_credentials_t credentials;
    strncpy(credentials.ssid, ctx->selected_ssid, sizeof(credentials.ssid) - 1);
    credentials.ssid[sizeof(credentials.ssid) - 1] = '\0';
    strncpy(credentials.password, password, sizeof(credentials.password) - 1);
    credentials.password[sizeof(credentials.password) - 1] = '\0';

    ESP_LOGI(SPLASH_TAG, "Submitting credentials - SSID: %s, Password length: %d",
             credentials.ssid, strlen(credentials.password));

    esp_err_t ret = save_wifi_config(credentials.ssid, credentials.password);
    if (ret == ESP_OK)
    {
        // Hide UI elements
        lv_obj_add_flag(ctx->password_popup, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(ctx->keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(ctx->debug_label, "Debug: WiFi credentials saved successfully. Reconnecting...");
        lv_obj_set_style_text_color(ctx->debug_label, lv_color_hex(0x00FF00), 0);

        // Clean up UI
        splash_cleanup_wifi_popup(ctx);
        splash_cleanup_wifi_list(ctx);

        ESP_LOGI(SPLASH_TAG, "Setting state to SPLASH_STATE_RECONNECT_WIFI");
        splash_screen_set_state(SPLASH_STATE_RECONNECT_WIFI);
    }
    else
    {
        ESP_LOGE(SPLASH_TAG, "Failed to save WiFi config: %s", esp_err_to_name(ret));
        lv_label_set_text(ctx->debug_label, "Debug: Failed to save WiFi credentials.");
        lv_obj_set_style_text_color(ctx->debug_label, lv_color_hex(0xFF0000), 0);
    }
}

void splash_cleanup_wifi_list(splash_ctx_t *ctx)
{
    if (ctx->networks_list)
    {
        // Free button data for each button
        uint32_t child_cnt = lv_obj_get_child_cnt(ctx->networks_list);
        for (uint32_t i = 0; i < child_cnt; i++)
        {
            lv_obj_t *child = lv_obj_get_child(ctx->networks_list, i);
            if (child)
            {
                // Get and free the allocated SSID copy
                void *ssid_copy = lv_obj_get_user_data(child);
                if (ssid_copy)
                {
                    ESP_LOGI(SPLASH_TAG, "Freeing SSID memory: %s", (char *)ssid_copy);
                    free(ssid_copy);
                }
            }
        }
        // Delete the entire list widget (this also deletes all child widgets)
        lv_obj_del(ctx->networks_list);
        ctx->networks_list = NULL;
    }
}

// Call this when closing the WiFi popup
void splash_cleanup_wifi_popup(splash_ctx_t *ctx)
{
    // Clear the selected SSID
    memset(ctx->selected_ssid, 0, sizeof(ctx->selected_ssid));

    // Hide the popup and keyboard
    if (ctx->password_popup)
    {
        lv_obj_add_flag(ctx->password_popup, LV_OBJ_FLAG_HIDDEN);
    }
    if (ctx->keyboard)
    {
        lv_obj_add_flag(ctx->keyboard, LV_OBJ_FLAG_HIDDEN);
    }

    // Clear any password input
    if (ctx->password_input)
    {
        lv_textarea_set_text(ctx->password_input, "");
    }
}