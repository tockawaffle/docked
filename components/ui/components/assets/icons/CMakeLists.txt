# Gather all .c source files in the current directory recursively
file(GLOB_RECURSE UI_SOURCES
    "*.c"
)

# Register this folder as a component
idf_component_register(
    SRCS ${UI_SOURCES}
    INCLUDE_DIRS "."  # Include the current directory (logos folder)
)
