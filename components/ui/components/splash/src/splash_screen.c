#include "splash_internal.h"

static splash_ctx_t ctx;

#define DEBUG_KEY 1

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
    { // Check if not already deleted
        ESP_LOGI(SPLASH_TAG, "Timer deleted by: %s", caller_function);
        lv_timer_del(timer);
    }
}

void splash_screen_init(lv_obj_t *main_screen)
{
    // Set the screen's background color
    lv_obj_set_style_bg_color(main_screen, lv_color_black(), LV_PART_MAIN);

    // Create a container to hold all elements
    lv_obj_t *cont = lv_obj_create(main_screen);
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create and setup logo
    lv_obj_t *gear_label = lv_label_create(cont);
    lv_obj_set_style_text_font(gear_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(gear_label, lv_color_hex(COLOR_PRIMARY), 0);
    lv_label_set_text(gear_label, LV_SYMBOL_SETTINGS);

    // Create and setup loading bar
    ctx.loading_bar = lv_bar_create(cont);
    lv_obj_set_size(ctx.loading_bar, 380, 20);
    lv_obj_set_style_bg_color(ctx.loading_bar, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_bar_set_value(ctx.loading_bar, 0, LV_ANIM_OFF);

    // Create and setup loading label
    lv_obj_t *loading_label = lv_label_create(cont);
    lv_label_set_text(loading_label, "I'm cooking, wait a sec...");
    lv_obj_set_style_text_color(loading_label, lv_color_hex(COLOR_MUTED), 0);

    // Create and setup debug label
    ctx.debug_label = lv_label_create(cont);
    lv_label_set_text(ctx.debug_label, "Debug: Starting initialization...");
    lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(COLOR_MUTED), 0);

    // Add spacing between elements
    lv_obj_set_style_pad_row(cont, 20, 0);

    // Initialize context
    ctx.state = SPLASH_INIT_SD_CARD;
    ctx.networks_list = NULL;
    ctx.password_popup = NULL;
    ctx.password_input = NULL;
    ctx.keyboard = NULL;
    ctx.timer_deleted = false;

    // Create a timer to handle the initialization steps
    lv_timer_create(splash_task_cb, 500, NULL);
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
        if (ret == ESP_OK)
        {
            ctx.state = SPLASH_INIT_WIFI;
            lv_bar_set_value(ctx.loading_bar, 25, LV_ANIM_ON);
        }
        else
        {
            ESP_LOGE(SPLASH_TAG, "Failed to initialize SD Card: %s", esp_err_to_name(ret));
            lv_label_set_text(ctx.debug_label, "Debug: Failed to initialize SD Card. Please check if there's any inserted.");
            lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0xFF0000), 0);

            ESP_LOGI(SPLASH_TAG, "Deleting timer at SPLASH_INIT_SD_CARD");
            // f
            return;
        }
        break;

    case SPLASH_INIT_WIFI:
        lv_label_set_text(ctx.debug_label, "Debug: Initializing Wi-Fi...");
        ret = wifi_init();
        if (ret == ESP_OK)
        {
            lv_bar_set_value(ctx.loading_bar, 50, LV_ANIM_ON);
            lv_label_set_text(ctx.debug_label, "Debug: Reading Wi-Fi configuration...");
            wifi_credentials_t wifi_credentials;
            ret = read_wifi_config(&wifi_credentials);

#if DEBUG_KEY
            ret = ESP_FAIL; // Force the error for testing
#endif

            if (ret != ESP_OK)
            {
                ESP_LOGE(SPLASH_TAG, "Failed to read Wi-Fi credentials: %s", esp_err_to_name(ret));
                lv_label_set_text(ctx.debug_label, "Debug: Scanning available networks...");
                wifi_scan_result_t scan_result = wifi_scan();
                if (scan_result.status != ESP_OK)
                {
                    ESP_LOGE(SPLASH_TAG, "Failed to scan Wi-Fi networks: %s", esp_err_to_name(ret));
                    lv_label_set_text(ctx.debug_label, "Debug: Failed to scan Wi-Fi networks.");
                    lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0xFF0000), 0);

                    ESP_LOGI(SPLASH_TAG, "Deleting timer at SPLASH_INIT_WIFI");
                    splash_delete_timer(timer, __FUNCTION__);
                    return;
                }
                else
                {
                    lv_label_set_text(ctx.debug_label, "Debug: Wi-Fi networks scanned successfully.");
                    lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0x00FF00), 0);
                    splash_wifi_create_network_list(&ctx, &scan_result);
                    splash_wifi_create_password_popup(&ctx);
                    ctx.state = SPLASH_WAIT_WIFI_INPUT;
                }
            }
            else
            {
                lv_label_set_text(ctx.debug_label, "Debug: Wi-Fi credentials read successfully.");
                lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0x00FF00), 0);

                ret = wifi_restart();
                if (ret != ESP_OK)
                {
                    ESP_LOGE(SPLASH_TAG, "Failed to restart Wi-Fi: %s", esp_err_to_name(ret));
                    lv_label_set_text(ctx.debug_label, "Debug: Failed to restart Wi-Fi.");
                    lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0xFF0000), 0);

                    ESP_LOGI(SPLASH_TAG, "Deleting timer at SPLASH_INIT_WIFI 2");
                    splash_delete_timer(timer, __FUNCTION__);
                    return;
                }
                else
                {
                    ctx.state = SPLASH_INIT_UI;
                }
                ctx.state = SPLASH_INIT_UI;
            }
        }
        else
        {
            ESP_LOGE(SPLASH_TAG, "Failed to initialize Wi-Fi: %s", esp_err_to_name(ret));
            lv_label_set_text(ctx.debug_label, "Debug: Failed to initialize Wi-Fi.");
            lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0xFF0000), 0);

            ESP_LOGI(SPLASH_TAG, "Deleting timer at SPLASH_INIT_WIFI 3");
            splash_delete_timer(timer, __FUNCTION__);
            return;
        }
        break;
    case SPLASH_STATE_RECONNECT_WIFI:
        ESP_LOGI(SPLASH_TAG, "Reconnecting to Wi-Fi...");
        lv_label_set_text(ctx.debug_label, "Debug: Reconnecting to Wi-Fi...");
        { // Added block to handle local variable declarations
            wifi_credentials_t wifi_credentials;
            ret = read_wifi_config(&wifi_credentials);
            if (ret != ESP_OK)
            {
                ESP_LOGE(SPLASH_TAG, "Failed to read Wi-Fi credentials: %s", esp_err_to_name(ret));
                lv_label_set_text(ctx.debug_label, "Debug: Failed to read Wi-Fi credentials. Please reconfigure.");
                lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0xFF0000), 0);

                // Maybe set the state to SPLASH_INIT_WIFI to retry?
                ctx.state = SPLASH_INIT_WIFI; // This might cause bugs, be aware!!!
            }
            else
            {
                ret = wifi_connect(wifi_credentials.ssid, wifi_credentials.password);
                if (ret == ESP_OK)
                {
                    lv_label_set_text(ctx.debug_label, "Debug: Reconnected to Wi-Fi successfully.");
                    lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0x00FF00), 0);
                    ctx.state = SPLASH_INIT_UI;
                }
                else
                {
                    ESP_LOGE(SPLASH_TAG, "Failed to reconnect to Wi-Fi: %s", esp_err_to_name(ret));
                    lv_label_set_text(ctx.debug_label, "Debug: Failed to reconnect to Wi-Fi.");
                    lv_obj_set_style_text_color(ctx.debug_label, lv_color_hex(0xFF0000), 0);

                    // We should stop the timer here, as we are in a failed state
                    ESP_LOGI(SPLASH_TAG, "Deleting timer at SPLASH_STATE_RECONNECT_WIFI");
                    splash_delete_timer(timer, __FUNCTION__);
                }
            }
        }
        break;
    case SPLASH_INIT_UI:
        lv_bar_set_value(ctx.loading_bar, 100, LV_ANIM_ON);
        ctx.state = SPLASH_INIT_DONE;
        ctx.timer_deleted = true;
        splash_delete_timer(timer, __FUNCTION__);
        break;
    case SPLASH_INIT_DONE:
        // Only delete the timer once when we reach DONE state
        ESP_LOGI(SPLASH_TAG, "Splash screen initialization complete");
        break;
    default:
        ESP_LOGW(SPLASH_TAG, "Unhandled state: %d", ctx.state);
        break;
    }
}
