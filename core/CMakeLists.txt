cmake_minimum_required(VERSION 3.13)
project(
    core
    DESCRIPTION "RP2040 Core Library"
    VERSION 0.4.1
    LANGUAGES C CXX ASM
)

# List of source files
set(SOURCE
    ${SOURCE}
    src/command/command.c
    src/console/console.c
    src/draw/bmpspritesheet.c
    src/draw/canvas.c
    src/drivers/adxl345.c
    src/drivers/at24c.c
    src/drivers/sht3xdis.c
    src/drivers/ssd1306.c
    src/json/cjson.c
    src/logger.c
    src/tools/i2c.c
    src/tools/weather.c
    src/types/hash_table.c
    )

# List of header files
set(HEADERS
    ${HEADERS}
    include/core/command/command.h
    include/core/console/console.h
    include/core/draw/bmpspritesheet.h
    include/core/draw/canvas.h
    include/core/drivers/adxl345.h
    include/core/drivers/at24c.h
    include/core/drivers/sht3xdis.h
    include/core/drivers/ssd1306.h
    include/core/json/cjson.h
    include/core/logger.h
    include/core/tools/i2c.h
    include/core/tools/weather.h
    include/core/types/hash_table.h
    include/core/version.h
    )

add_library(
    ${PROJECT_NAME} STATIC
    ${SOURCE}
    ${HEADERS}
    )

target_compile_definitions(${PROJECT_NAME} PUBLIC CORE_VERSION="${PROJECT_VERSION}")

# pico_generate_pio_header(
#     ${PROJECT_NAME}
#         ${CMAKE_CURRENT_LIST_DIR}/pio/ws2812.pio
#     UTPUT_DIR
#         ${CMAKE_CURRENT_LIST_DIR}/generated
# )

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
