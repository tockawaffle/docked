#include "./ports/lcd_port.h"
#include "ui.h"
#include "sd_card/sd_card.h"
#include "wifi/wifi.h"

void app_main()
{
    ESP_ERROR_CHECK(lcd_init());
    vTaskDelay(pdMS_TO_TICKS(500));

    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1))
    {
        // ESP_ERROR_CHECK(sd_card_init());

        // ESP_ERROR_CHECK(wifi_init());
        // ESP_ERROR_CHECK(wifi_connect("Guarufix Frente 2G", "Guaru0308@2g"));
        
        
        // Creates the UI
        create_ui();
        // Release the mutex
        lvgl_port_unlock();
    }
}
