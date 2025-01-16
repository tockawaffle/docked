#include "splash_internal.h"

static splash_ctx_t ctx;
static void splash_task_cb(lv_timer_t *timer);

splash_ctx_t *splash_screen_get_context(void)
{
    return &ctx;
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

    // Create a timer to handle the initialization steps
    lv_timer_create(splash_task_cb, 500, NULL);
}

static void splash_task_cb(lv_timer_t *timer)
{
    esp_err_t ret;

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
            lv_timer_del(timer);
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
            ESP_LOGI(SPLASH_TAG, "SSID: %s", wifi_credentials.ssid);
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
                    lv_timer_del(timer);
                }
                else
                {
                    lv_label_set_text(ctx.debug_label, "Debug: Wi-Fi networks scanned successfully.");
                    splash_wifi_create_network_list(&ctx, &scan_result);
                    splash_wifi_create_password_popup(&ctx);
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
                    lv_timer_del(timer);
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
            lv_timer_del(timer);
        }
        break;

    case SPLASH_INIT_UI:
        lv_bar_set_value(ctx.loading_bar, 100, LV_ANIM_ON);
        ctx.state = SPLASH_INIT_DONE;
        lv_timer_del(timer);
        break;

    default:
        break;
    }
}
