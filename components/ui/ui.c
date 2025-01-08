#include "ui.h"
static const char *TAG = "ui.c";

static lv_obj_t *main_screen;
static lv_obj_t *counter_label;
static lv_obj_t *inc_btn;
static lv_obj_t *dec_btn;
static int counter_value = 0;

static void increment_button_cb(lv_event_t *e)
{
    ESP_LOGI(TAG, "Increment pressed, counter_label ptr: %p", counter_label);
    counter_value++;
    
    if (!counter_label || !lv_obj_is_valid(counter_label)) {
        ESP_LOGE(TAG, "Counter label invalid");
        return;
    }
    
    lv_label_set_text_fmt(counter_label, "Count: %d", counter_value);
    ESP_LOGI(TAG, "Text updated, checking label valid: %d", lv_obj_is_valid(counter_label));
}

static void decrement_button_cb(lv_event_t *e)
{
    counter_value--;
    lv_label_set_text_fmt(counter_label, "Count: %d", counter_value);
}

// Creates and handles the main UI.
void create_ui(void)
{
    // Create main screen
    main_screen = lv_obj_create(NULL);
    lv_scr_load(main_screen);

    // Create title label
    lv_obj_t *title = lv_label_create(main_screen);
    lv_label_set_text(title, "LVGL Test UI");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Create counter label
    counter_label = lv_label_create(main_screen);
    lv_label_set_text_fmt(counter_label, "Count: %d", counter_value);
    lv_obj_align(counter_label, LV_ALIGN_CENTER, 0, 0);

    // Create increment button
    inc_btn = lv_btn_create(main_screen);
    lv_obj_add_event_cb(inc_btn, increment_button_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_align(inc_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_t *inc_label = lv_label_create(inc_btn);
    lv_label_set_text(inc_label, "+");
    lv_obj_center(inc_label);

    // Create decrement button
    dec_btn = lv_btn_create(main_screen);
    lv_obj_add_event_cb(dec_btn, decrement_button_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_align(dec_btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_t *dec_label = lv_label_create(dec_btn);
    lv_label_set_text(dec_label, "-");
    lv_obj_center(dec_label);
}
