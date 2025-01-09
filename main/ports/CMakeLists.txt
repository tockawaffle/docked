set(srcs
    "lcd_port.c"
    "lvgl_port.c"
)

set(includes
    "."
)

target_sources(${COMPONENT_LIB} PRIVATE ${srcs})
target_include_directories(${COMPONENT_LIB} PUBLIC ${includes})