cmake_minimum_required(VERSION 3.13)
project(
    common
    DESCRIPTION "RP2040 Common Library"
    VERSION 0.0.1
    LANGUAGES C CXX ASM
)

# List of source files
set(SOURCE
    ${SOURCE}
    src/console/console.cpp
    src/logger.cpp
    )   

# List of header files
set(HEADERS
    ${HEADERS}
    include/common/console/console.h
    include/common/logger.h
    )   

add_library(
    ${PROJECT_NAME} STATIC
    ${SOURCE}
    ${HEADERS}
    )   

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        pico_stdlib
    )

# Specifies include directories to use when compiling a given target
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )   

# Create the __FILENAME__ macro for use with the logger
define_file_basename_for_sources(${PROJECT_NAME})
