#include "./ports/lcd_port.h"
#include "ui.h"

void app_main()
{
    lcd_init(); // Initialize the Waveshare ESP32-S3 RGB LCD 
    // wavesahre_rgb_lcd_bl_on();  //Turn on the screen backlight 
    // wavesahre_rgb_lcd_bl_off(); //Turn off the screen backlight 

    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(-1)) {
        // Creates the UI
        create_ui();
        // Release the mutex
        lvgl_port_unlock();
    }
}
