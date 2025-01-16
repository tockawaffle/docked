#include "splash_internal.h"

void splash_wifi_create_password_popup(splash_ctx_t *ctx)
{
    if (ctx->password_popup == NULL)
    {
        // Create the popup container
        ctx->password_popup = lv_obj_create(lv_scr_act());
        lv_obj_set_size(ctx->password_popup, LV_PCT(90), LV_PCT(50));
        lv_obj_align(ctx->password_popup, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(ctx->password_popup, lv_color_hex(0x202020), 0);
        lv_obj_set_style_bg_opa(ctx->password_popup, LV_OPA_90, 0);
        lv_obj_set_style_border_width(ctx->password_popup, 2, 0);
        lv_obj_set_style_border_color(ctx->password_popup, lv_color_hex(COLOR_PRIMARY), 0);
        lv_obj_set_style_radius(ctx->password_popup, 10, 0);
        lv_obj_set_style_pad_all(ctx->password_popup, 20, 0);

        // Close button
        lv_obj_t *close_btn = lv_btn_create(ctx->password_popup);
        lv_obj_set_size(close_btn, 30, 30);
        lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, -10, 10);
        lv_obj_set_style_bg_color(close_btn, lv_color_hex(0xFF0000), 0);
        lv_obj_set_style_radius(close_btn, LV_RADIUS_CIRCLE, 0);

        lv_obj_t *close_label = lv_label_create(close_btn);
        lv_label_set_text(close_label, LV_SYMBOL_CLOSE);
        lv_obj_center(close_label);

        lv_obj_add_event_cb(close_btn, splash_on_close_popup, LV_EVENT_CLICKED, NULL);

        // Title
        lv_obj_t *title = lv_label_create(ctx->password_popup);
        lv_label_set_text(title, "WiFi Connection");
        lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
        lv_obj_set_style_text_color(title, lv_color_hex(COLOR_PRIMARY), 0);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

        // Selected network label
        lv_obj_t *ssid_label = lv_label_create(ctx->password_popup);
        lv_obj_set_style_text_color(ssid_label, lv_color_hex(COLOR_MUTED), 0);
        lv_label_set_text(ssid_label, "Network: None selected");
        lv_obj_align(ssid_label, LV_ALIGN_TOP_LEFT, 0, 50);

        // Password input container
        lv_obj_t *pwd_cont = lv_obj_create(ctx->password_popup);
        lv_obj_remove_style_all(pwd_cont);
        lv_obj_set_size(pwd_cont, LV_PCT(100), 45);
        lv_obj_align(pwd_cont, LV_ALIGN_TOP_LEFT, 0, 80);
        lv_obj_set_flex_flow(pwd_cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(pwd_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_gap(pwd_cont, 10, 0);

        // Password input field
        ctx->password_input = lv_textarea_create(pwd_cont);
        lv_textarea_set_password_mode(ctx->password_input, true);
        lv_textarea_set_one_line(ctx->password_input, true);
        lv_textarea_set_placeholder_text(ctx->password_input, "Enter password");
        lv_obj_set_size(ctx->password_input, LV_PCT(80), 45);
        lv_obj_add_event_cb(ctx->password_input, splash_on_text_area_event, LV_EVENT_ALL, NULL);

        // Show/Hide password button
        lv_obj_t *show_pwd_btn = lv_btn_create(pwd_cont);
        lv_obj_set_size(show_pwd_btn, 45, 45);
        lv_obj_t *show_label = lv_label_create(show_pwd_btn);
        lv_label_set_text(show_label, LV_SYMBOL_EYE_OPEN);
        lv_obj_center(show_label);
        lv_obj_add_event_cb(show_pwd_btn, splash_on_password_toggle, LV_EVENT_CLICKED, NULL);

        // Connect button
        ctx->connect_btn = lv_btn_create(ctx->password_popup);
        lv_obj_set_size(ctx->connect_btn, LV_PCT(80), 40);
        lv_obj_align(ctx->connect_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
        lv_obj_set_style_bg_color(ctx->connect_btn, lv_color_hex(COLOR_PRIMARY), 0);

        lv_obj_t *connect_label = lv_label_create(ctx->connect_btn);
        lv_label_set_text(connect_label, "Connect");
        lv_obj_center(connect_label);

        // Add event callback
        lv_obj_add_event_cb(ctx->connect_btn, splash_on_password_submit, LV_EVENT_CLICKED, NULL);

        lv_obj_add_event_cb(ctx->connect_btn, splash_on_password_submit, LV_EVENT_CLICKED, NULL);

        splash_keyboard_init(ctx);

        // Initially hide the popup
        lv_obj_add_flag(ctx->password_popup, LV_OBJ_FLAG_HIDDEN);
    }
}

void splash_keyboard_init(splash_ctx_t *ctx)
{
    if (ctx->keyboard == NULL)
    {
        ctx->keyboard = lv_keyboard_create(lv_scr_act());
        lv_obj_set_size(ctx->keyboard, LV_PCT(100), LV_PCT(50));
        lv_obj_align(ctx->keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);

        // Set keyboard mode and style
        lv_keyboard_set_mode(ctx->keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        lv_obj_set_style_bg_color(ctx->keyboard, lv_color_hex(0x303030), 0);
        lv_obj_set_style_border_color(ctx->keyboard, lv_color_hex(COLOR_PRIMARY), 0);
        lv_obj_set_style_border_width(ctx->keyboard, 1, 0);

        // Connect to textarea
        lv_keyboard_set_textarea(ctx->keyboard, ctx->password_input);

        // Add event handlers
        lv_obj_add_event_cb(ctx->password_input, splash_on_keyboard_input, LV_EVENT_ALL, NULL);

        // Initially hide keyboard
        lv_obj_add_flag(ctx->keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

void splash_keyboard_show(splash_ctx_t *ctx)
{
    if (ctx->keyboard != NULL)
    {
        lv_keyboard_set_textarea(ctx->keyboard, ctx->password_input);
        lv_obj_clear_flag(ctx->keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

void splash_keyboard_hide(splash_ctx_t *ctx)
{
    if (ctx->keyboard != NULL)
    {
        lv_obj_add_flag(ctx->keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

void splash_on_text_area_event(lv_event_t *e)
{
    splash_ctx_t *ctx = splash_screen_get_context();
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_FOCUSED)
    {
        splash_keyboard_show(ctx);
    }
    else if (code == LV_EVENT_DEFOCUSED)
    {
        splash_keyboard_hide(ctx);
    }
}

void splash_on_password_toggle(lv_event_t *e)
{
    splash_ctx_t *ctx = splash_screen_get_context();
    bool is_hidden = lv_textarea_get_password_mode(ctx->password_input);
    lv_textarea_set_password_mode(ctx->password_input, !is_hidden);

    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text(label, is_hidden ? LV_SYMBOL_EYE_CLOSE : LV_SYMBOL_EYE_OPEN);
}

void splash_on_close_popup(lv_event_t *e)
{
    splash_ctx_t *ctx = splash_screen_get_context();
    lv_obj_add_flag(ctx->password_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ctx->keyboard, LV_OBJ_FLAG_HIDDEN);
}

// Focus management for the keyboard
void splash_keyboard_focus_handler(lv_event_t *e, bool should_show)
{
    splash_ctx_t *ctx = splash_screen_get_context();
    if (should_show)
    {
        lv_keyboard_set_textarea(ctx->keyboard, ctx->password_input);
        lv_obj_clear_flag(ctx->keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(ctx->keyboard);
    }
    else
    {
        lv_obj_add_flag(ctx->keyboard, LV_OBJ_FLAG_HIDDEN);
    }
}

// Event handler for textarea input
void splash_on_keyboard_input(lv_event_t *e)
{
    splash_ctx_t *ctx = splash_screen_get_context();
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        splash_keyboard_focus_handler(e, true);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED); // Remove focus indicator
    }
    else if (code == LV_EVENT_DEFOCUSED)
    {
        splash_keyboard_focus_handler(e, false);
    }
    else if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
    {
        splash_keyboard_hide(ctx);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
    }
    // Handle keyboard submit event
    else if (code == LV_EVENT_VALUE_CHANGED)
    {
        const char *txt = lv_textarea_get_text(ta);
        // You could add password validation here if needed
        if (strlen(txt) > 0)
        {
            lv_obj_add_state(ctx->connect_btn, LV_STATE_DEFAULT);
        }
        else
        {
            lv_obj_clear_state(ctx->connect_btn, LV_STATE_DEFAULT);
        }
    }
}

// Helper function to clear the password input
void splash_keyboard_clear_input(splash_ctx_t *ctx)
{
    if (ctx->password_input != NULL)
    {
        lv_textarea_set_text(ctx->password_input, "");
        lv_obj_clear_state(ctx->password_input, LV_STATE_FOCUSED);
    }
    splash_keyboard_hide(ctx);
}