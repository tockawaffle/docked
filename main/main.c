#include "./ports/lcd_port.h"
#include "ui.h"

void app_main()
{
    ESP_ERROR_CHECK(lcd_init());
    vTaskDelay(pdMS_TO_TICKS(500));

    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1))
    {   
        // Creates the UI
        create_ui();
        // Release the mutex
        lvgl_port_unlock();
    }
}
