idf_component_register(
    SRCS
        "main.c"
        "ports/lcd_port.c"
        "ports/lvgl_port.c"
    INCLUDE_DIRS
        "."
        "ports"
        "include"
    REQUIRES
        lvgl
        esp_lcd_touch
        esp_lcd_touch_gt911
        ui
        server
        wifi
        storage
)

# Add optimization flags
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
)

if(TARGET lvgl)
    idf_component_get_property(lvgl_lib lvgl COMPONENT_LIB)
    target_compile_options(${lvgl_lib} PRIVATE -Wno-format)
endif()
