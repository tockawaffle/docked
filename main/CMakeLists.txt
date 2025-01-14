idf_component_register(
    SRCS
    "main.c"
    "ports/lcd_port.c"
    "ports/lvgl_port.c"
    "sd_card/sd_card.c"
    "sd_card/lv_fs.c"
    "wifi/wifi.c"
    INCLUDE_DIRS
    "."
    "ports"
    "sd_card"
    "wifi"
    REQUIRES
    lvgl
    esp_lcd_touch
    esp_lcd_touch_gt911
    ui
    fatfs
    vfs
    sdmmc
    freertos
    esp_wifi
    esp_netif
    nvs_flash
)

# Add optimization flags
target_compile_options(${COMPONENT_LIB} PRIVATE 
    -Os
    -ffunction-sections
    -fdata-sections
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
)

idf_component_get_property(lvgl_lib lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)