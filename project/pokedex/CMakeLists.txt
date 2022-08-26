cmake_minimum_required(VERSION 3.13)
project(
    pokedex
    DESCRIPTION "Pokedex Project Library"
    VERSION 0.0.1
    LANGUAGES C CXX ASM
)

set( SOURCE
    ${SOURCE}
        src/application.cpp
        src/bmpspritesheet.cpp
        src/draw/canvas.cpp
        src/draw/draw.cpp
        src/draw/font.cpp
        src/resources.cpp
        src/resources/red_blue.bmp.s
)

set( HEADERS
    ${HEADERS}
        include/project/application.h
        include/project/bmpspritesheet.h
        include/project/draw/canvas.h
        include/project/draw/draw.h
        include/project/draw/font.h
        include/project/resources.h
)

add_library(
    ${PROJECT_NAME} STATIC
    ${SOURCE}
    ${HEADERS}
)

target_compile_definitions(
    ${PROJECT_NAME} PRIVATE 
        POKEDEX_VERSION="${PROJECT_VERSION}"
        )

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        common
        pico_multicore
    )

# Specifies include directories to use when compiling a given target
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )   

# Create the __FILENAME__ macro for use with the logger
define_file_basename_for_sources(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CONTROL_LIB_DIR}
    ARCHIVE DESTINATION ${CONTROL_LIB_DIR}
    )  
