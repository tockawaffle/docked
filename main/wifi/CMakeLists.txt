set(srcs
    "wifi.c"
)

set(includes
    "."
)

target_sources(${COMPONENT_LIB} PRIVATE ${srcs})
target_include_directories(${COMPONENT_LIB} PUBLIC ${includes})