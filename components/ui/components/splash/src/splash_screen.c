#include "splash_internal.h"

static splash_ctx_t ctx;
#define DEBUG_KEY 0

splash_ctx_t *splash_screen_get_context(void)
{
    return &ctx;
}

splash_init_state_t splash_screen_set_state(splash_init_state_t new_ctx)
{
    splash_init_state_t old_state = ctx.state;
    ctx.state = new_ctx;
    ESP_LOGI(SPLASH_TAG, "State changed from %d to %d", old_state, new_ctx);
    return old_state;
}

void splash_delete_timer(lv_timer_t *timer, const char *caller_function)
{
    if (timer && !ctx.timer_deleted)
    {
        ESP_LOGI(SPLASH_TAG, "Timer deleted by: %s", caller_function);
        lv_timer_del(timer);
    }
}

void splash_screen_init()
{
    lv_obj_t *splash_screen = lv_obj_create(NULL);
    lv_scr_load(splash_screen);
    lv_obj_set_style_bg_color(splash_screen, lv_color_hex(COLOR_SECONDARY), LV_PART_MAIN);

    ctx.splash_screen = splash_screen;

    lv_obj_t *cont = lv_obj_create(ctx.splash_screen);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 20, 0);

    lv_obj_t *logo_img = lv_img_create(cont);
    lv_img_set_src(logo_img, &logo);
    lv_obj_set_size(logo_img, 200, 200);


    ctx.loading_bar = lv_bar_create(cont);
    lv_obj_set_size(ctx.loading_bar, 380, 20);
    lv_obj_set_style_bg_color(ctx.loading_bar, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_bar_set_value(ctx.loading_bar, 0, LV_ANIM_OFF);

    lv_obj_t *loading_label = lv_label_create(cont);
    lv_label_set_text(loading_label, "I'm cooking, wait a sec...");
    lv_obj_set_style_text_color(loading_label, lv_color_hex(COLOR_MUTED), 0);

    ctx.debug_label = lv_label_create(cont);
    lv_label_set_text(ctx.debug_label, "Debug: Starting initialization...");
    lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(COLOR_MUTED), 0);

    ctx.state = SPLASH_INIT_SD_CARD;
    ctx.networks_list = NULL;
    ctx.password_popup = NULL;
    ctx.password_input = NULL;
    ctx.keyboard = NULL;
    ctx.retry_btn = NULL;
    ctx.timer_deleted = false;
    ctx.selected_ssid[0] = '\0';

    lv_timer_create(splash_task_cb, 500, NULL);
}

void splash_handle_wifi_retry(lv_event_t *e)
{
    ctx.state = SPLASH_INIT_WIFI;
    if (ctx.timer_deleted)
    {
        ctx.timer_deleted = false;
        lv_timer_create(splash_task_cb, 500, NULL);
    }
}

void splash_handle_scan_networks(lv_event_t *e)
{
    wifi_scan_result_t scan_result = wifi_scan();
    if (scan_result.status != ESP_OK)
    {
        lv_label_set_text(ctx.debug_label, "Debug: Failed to scan networks. Please try again.");
        return;
    }

    splash_wifi_create_network_list(&ctx, &scan_result);
    splash_wifi_create_password_popup(&ctx);
    ctx.state = SPLASH_WAIT_WIFI_INPUT;
}

void splash_task_cb(lv_timer_t *timer)
{
    if (ctx.timer_deleted)
    {
        return;
    }

    esp_err_t ret;
    ESP_LOGI(SPLASH_TAG, "Current state: %d", ctx.state);

    switch (ctx.state)
    {
    case SPLASH_INIT_SD_CARD:
        lv_label_set_text(ctx.debug_label, "Debug: Initializing SD Card...");
        ret = sd_card_init();

        if (ret != ESP_OK)
        {
            ESP_LOGE(SPLASH_TAG, "Failed to initialize SD Card: %s", esp_err_to_name(ret));
            lv_label_set_text(ctx.debug_label, "Debug: Failed to initialize SD Card. Please check if card is inserted.");
            lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0xFF0000), 0);
            goto cleanup;
        }

        lv_bar_set_value(ctx.loading_bar, 25, LV_ANIM_ON);
        ctx.state = SPLASH_INIT_WIFI;
        break;

    case SPLASH_INIT_WIFI:
    {
        wifi_detailed_status_t status = {0};
        lv_label_set_text(ctx.debug_label, "Debug: Initializing Wi-Fi...");
        ret = wifi_init();

        if (ret != ESP_OK)
        {
            lv_label_set_text_fmt(ctx.debug_label, "Debug: Failed to initialize Wi-Fi: %s", esp_err_to_name(ret));
            lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0xFF0000), 0);
            goto cleanup;
        }

        lv_bar_set_value(ctx.loading_bar, 50, LV_ANIM_ON);
        lv_label_set_text(ctx.debug_label, "Debug: Reading Wi-Fi configuration...");

        wifi_credentials_t wifi_credentials;
        ret = read_wifi_config(&wifi_credentials);

#if DEBUG_KEY == 1
        ret = ESP_FAIL;
#endif

        if (ret != ESP_OK)
        {
            splash_handle_scan_networks(NULL);
            break;
        }

        status = wifi_connect(wifi_credentials.ssid, wifi_credentials.password);
        if (status.code != ESP_OK)
        {
            lv_label_set_text_fmt(ctx.debug_label, "Debug: Failed to connect to %s: %s",
                                  wifi_credentials.ssid, status.message);
            lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0xFF0000), 0);

            if (!ctx.retry_btn)
            {
                lv_obj_t *parent = lv_obj_get_parent(ctx.debug_label);
                ctx.retry_btn = lv_btn_create(parent);
                lv_obj_t *btn_label = lv_label_create(ctx.retry_btn);
                lv_label_set_text(btn_label, "Try Again");
                lv_obj_center(btn_label);
                lv_obj_add_event_cb(ctx.retry_btn, splash_handle_scan_networks, LV_EVENT_CLICKED, NULL);
            }
            lv_obj_clear_flag(ctx.retry_btn, LV_OBJ_FLAG_HIDDEN);
            goto cleanup;
        }

        lv_label_set_text_fmt(ctx.debug_label, "Debug: Connected to %s successfully", wifi_credentials.ssid);
        lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0x00FF00), 0);
        ctx.state = SPLASH_INIT_UI;
        break;
    }

    case SPLASH_INIT_UI:
        lv_bar_set_value(ctx.loading_bar, 100, LV_ANIM_ON);
        splash_screen_set_state(SPLASH_INIT_DONE);

        break;

    case SPLASH_INIT_DONE:
        // Now we start the cleanup process and remove the splash screen
        lv_obj_del(ctx.splash_screen);
        // Free the ctx object
        memset(&ctx, 0, sizeof(ctx));
        // Stop the timer
        splash_delete_timer(timer, __FUNCTION__);

        // Start the main UI
        main_screen_init();

        break;

    case SPLASH_WAIT_WIFI_INPUT:
        break;

    default:
        ESP_LOGW(SPLASH_TAG, "Unhandled state: %d", ctx.state);
        break;
    }
    return;

cleanup:
    ctx.timer_deleted = true;
    splash_delete_timer(timer, __FUNCTION__);
}