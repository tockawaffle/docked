file(GLOB_RECURSE UI_SOURCES 
    "*.c"
    "components/*/*.c"
)

idf_component_register(
    SRCS ${UI_SOURCES}
    INCLUDE_DIRS "." "components"
    REQUIRES lvgl
)