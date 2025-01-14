#include "splash.h"

// Steps for splash screen
// 1. Initializes the SD Card
// 2. Initializes the Wi-Fi
typedef enum
{
    SPLASH_INIT_SD_CARD,
    SPLASH_INIT_WIFI,
    SPLASH_INIT_UI,
    SPLASH_INIT_DONE
} splash_init_state_t;

static splash_init_state_t splash_init_state = SPLASH_INIT_SD_CARD;

void splash_screen(lv_obj_t *main_screen)
{
    // Set the screen's background color if desired
    lv_obj_set_style_bg_color(main_screen, lv_color_black(), LV_PART_MAIN);

    // Create a container to hold all elements
    lv_obj_t *cont = lv_obj_create(main_screen);
    lv_obj_remove_style_all(cont);  // Remove background
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);  // Center the container
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Create and setup logo
    lv_obj_t *logo_ld = lv_img_create(cont);
    lv_img_set_src(logo_ld, &logo);
    lv_obj_set_size(logo_ld, 200, 200);
    lv_obj_center(logo_ld);

    // Create and setup loading bar
    lv_obj_t *loading_bar = lv_bar_create(cont);
    lv_obj_set_size(loading_bar, 380, 20);
    lv_obj_set_style_bg_color(loading_bar, lv_color_hex(COLOR_PRIMARY), LV_PART_MAIN);
    lv_bar_set_value(loading_bar, 0, LV_ANIM_OFF);
    
    // Create and setup loading label
    lv_obj_t *loading_label = lv_label_create(cont);
    lv_label_set_text(loading_label, "I'm cooking, wait a sec...");
    lv_obj_set_style_text_color(loading_label, lv_color_hex(COLOR_MUTED), 0);

    // Create and setup debug label
    lv_obj_t *debug_label = lv_label_create(cont);
    lv_label_set_text(debug_label, "Debug: Initializing SD Card...");
    lv_obj_set_style_text_color(debug_label, lv_color_hex(COLOR_MUTED), 0);

    // Add some spacing between elements
    lv_obj_set_style_pad_row(cont, 20, 0);  // Add 20px spacing between flex items

    // Initialize the SD Card
    if (splash_init_state == SPLASH_INIT_SD_CARD)
    {
        esp_err_t ret = sd_card_init();
        if (ret == ESP_OK)
        {
            vTaskDelay(pdMS_TO_TICKS(2000));
            splash_init_state = SPLASH_INIT_WIFI;
            lv_label_set_text(debug_label, "Debug: Initializing Wi-Fi...");
            lv_bar_set_value(loading_bar, 25, LV_ANIM_ON);
        }
        else
        {
            ESP_LOGE(SPLASH_TAG, "Failed to initialize SD Card: %s", esp_err_to_name(ret));
            lv_label_set_text(debug_label, "Debug: Failed to initialize SD Card. Please check if there's any inserted.");
            lv_obj_set_style_text_color(debug_label, lv_color_hex(0xFF0000), 0);
        }
    }
}