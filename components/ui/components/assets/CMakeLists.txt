# Gather all .c files in the parent folder recursively
file(GLOB_RECURSE PARENT_SOURCES
    "*.c"
)

# Register the parent component
idf_component_register(
    SRCS ${PARENT_SOURCES}
    INCLUDE_DIRS
        "."                # Include the parent directory
        "logos"            # Explicitly include the logos folder
        "icons"
    REQUIRES
        lvgl
        main
)
