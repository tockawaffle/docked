idf_component_register(
    SRCS 
        "waveshare_rgb_lcd_port.c" 
        "main.c"
        "lvgl_port.c"
    INCLUDE_DIRS "."
    REQUIRES
        lvgl
        esp_lcd_touch 
        esp_lcd_touch_gt911
        ui
)

idf_component_get_property(lvgl_lib lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
