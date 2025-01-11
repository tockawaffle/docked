idf_component_register(
    SRCS
    "main.c"
    "ports/lcd_port.c"
    "ports/lvgl_port.c"
    "sd_card/sd_card.c"
    INCLUDE_DIRS
    "."
    "ports"
    REQUIRES
    lvgl
    esp_lcd_touch
    esp_lcd_touch_gt911
    ui
    fatfs
    vfs
    sdmmc
)

idf_component_get_property(lvgl_lib lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
