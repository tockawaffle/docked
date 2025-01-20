#pragma once

#include <lvgl.h>

typedef enum {
    SPLASH_INIT_SD_CARD,
    SPLASH_INIT_WIFI,
    SPLASH_WAIT_WIFI_INPUT,
    SPLASH_STATE_RECONNECT_WIFI,
    SPLASH_INIT_UI,
    SPLASH_INIT_DONE
} splash_init_state_t;

typedef struct {
    lv_obj_t *loading_bar;
    lv_obj_t *debug_label;
    lv_obj_t *networks_list;  
    lv_obj_t *password_popup; 
    lv_obj_t *password_input; 
    lv_obj_t *keyboard;      
    lv_obj_t *connect_btn;    // Added connect button reference
    char selected_ssid[33];   
    splash_init_state_t state;
    bool timer_deleted;
} splash_ctx_t;