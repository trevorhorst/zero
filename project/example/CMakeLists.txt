cmake_minimum_required(VERSION 3.5)
project(
    example
    DESCRIPTION "Example Project"
    VERSION 0.0.1
    LANGUAGES C ASM
)

set(
    SOURCE
    ${SOURCE}
    src/application.c
)

set(
    HEADERS
    ${HEADERS}
    include/project/application.h
)

add_library(
    ${PROJECT_NAME} STATIC
    ${SOURCE}
    ${HEADERS}
)

target_compile_definitions(
    ${PROJECT_NAME} 
    PRIVATE
        EXAMPLE_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        core
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
