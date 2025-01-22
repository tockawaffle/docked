file(GLOB_RECURSE UI_SOURCES
    "*.c"
    "components/*/*.c"
)

idf_component_register(
    SRCS ${UI_SOURCES}
    INCLUDE_DIRS 
        "."
        "components/splash/include"
        "components/splash/private"
        "components/main/include"
        "components/main/private"
        "components/assets/"
        "components/assets/logos"
    REQUIRES
        lvgl
        main
)