cmake_minimum_required(VERSION 3.13)
project(
    epaper
    DESCRIPTION "E Paper Project Library"
    VERSION 1.0.0
    LANGUAGES C CXX ASM
    )   

# List of source files
set( SOURCE
    ${SOURCE}
    src/application.cpp
    src/command/command_facts.cpp
    src/draw/draw.cpp
    src/draw/font.cpp
    src/facts.cpp
    )   

# List of header files
set( HEADERS
    ${HEADERS}
    include/project/application.h
    include/project/command/command_facts.h
    include/project/draw/draw.h
    include/project/draw/font.h
    include/project/facts.h
    )   

add_library(
    ${PROJECT_NAME} STATIC
    ${SOURCE}
    ${HEADERS}
    )   

target_compile_definitions(${PROJECT_NAME} PRIVATE EPAPER_VERSION="${PROJECT_VERSION}")

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
