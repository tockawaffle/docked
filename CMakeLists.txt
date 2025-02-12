cmake_minimum_required(VERSION 3.16)

# Read version from file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" PROJECT_VER)
string(STRIP "${PROJECT_VER}" PROJECT_VER)

# Parse version components
string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ "${PROJECT_VER}")
set(VERSION_MAJOR ${CMAKE_MATCH_1})
set(VERSION_MINOR ${CMAKE_MATCH_2})
set(VERSION_PATCH ${CMAKE_MATCH_3})

# Generate files BEFORE including ESP-IDF build system
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/main/include/version.h
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/main/idf_component.yml.in
    ${CMAKE_CURRENT_SOURCE_DIR}/main/idf_component.yml
    @ONLY
)

# Now include ESP-IDF build system and set up project
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Set project version when declaring project
project(docked VERSION ${PROJECT_VER})