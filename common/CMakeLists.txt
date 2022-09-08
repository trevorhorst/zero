cmake_minimum_required(VERSION 3.13)
project(
    common
    DESCRIPTION "RP2040 Common Library"
    VERSION 0.3.7
    LANGUAGES C CXX ASM
)

# List of source files
set(SOURCE
    ${SOURCE}
    src/console/console.cpp
    src/command/command.cpp
    src/command/command_handler.cpp
    src/command/command_help.cpp
    src/control/control.cpp
    src/draw/bmpspritesheet.cpp
    src/draw/canvas.cpp
    src/drivers/epaper.cpp
    src/drivers/ssd1306.cpp
    src/drivers/ws2812.cpp
    src/json/cjson.cpp
    src/logger.cpp
    src/types.cpp
    )   

# List of header files
set(HEADERS
    ${HEADERS}
    include/common/console/console.h
    include/common/logger.h
    include/common/command/command.h
    include/common/command/command_handler.h
    include/common/command/command_help.h
    include/common/command/command_template.h
    include/common/control/control.h
    include/common/control/control_template.h
    include/common/draw/bmpspritesheet.h
    include/common/draw/canvas.h
    include/common/drivers/epaper.h
    include/common/drivers/ssd1306.h
    include/common/drivers/ws2812.h
    include/common/json/cjson.h
    include/common/types.h
    include/common/version.h
    )   

add_library(
    ${PROJECT_NAME} STATIC
    ${SOURCE}
    ${HEADERS}
    )   

target_compile_definitions(${PROJECT_NAME} PUBLIC COMMON_VERSION="${PROJECT_VERSION}")

pico_generate_pio_header(
    ${PROJECT_NAME} 
        ${CMAKE_CURRENT_LIST_DIR}/pio/ws2812.pio
    UTPUT_DIR 
        ${CMAKE_CURRENT_LIST_DIR}/generated
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        pico_stdlib
        hardware_spi
        hardware_i2c
        hardware_pio
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
