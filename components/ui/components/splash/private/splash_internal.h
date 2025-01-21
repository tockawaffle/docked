#pragma once

#include "splash_screen.h"
#include "splash_types.h"
#include <colors.h>
#include <sd_card.h>
#include <wifi.h>
#include <wifi_config.h>

#define SPLASH_TAG "SplashScreen"

void splash_task_cb(lv_timer_t *timer);
void splash_wifi_create_network_list(splash_ctx_t *ctx, wifi_scan_result_t *scan_result);
void splash_wifi_create_password_popup(splash_ctx_t *ctx);
void splash_cleanup_wifi_list(splash_ctx_t *ctx);
void splash_handle_wifi_retry(lv_event_t *e);
void splash_handle_scan_networks(lv_event_t *e);
void splash_on_wifi_button_click(lv_event_t *e);
void splash_on_password_submit(lv_event_t *e);
void splash_on_password_toggle(lv_event_t *e);
void splash_on_text_area_event(lv_event_t *e);
void splash_on_close_popup(lv_event_t *e);