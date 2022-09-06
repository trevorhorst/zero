cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)
include(pico-sdk/tools/CMakeLists.txt)

# Creates a portable way to implement the __FILENAME__ macro
# https://stackoverflow.com/questions/1706346/file-macro-manipulation-handling-at-compile-time
function(define_file_basename_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        # Add the FILE_BASENAME=filename compile definition to the list.
        get_filename_component(basename "${sourcefile}" NAME)
        # Set the updated compile definitions on the source file.
        set_property(
            SOURCE "${sourcefile}" APPEND
            PROPERTY COMPILE_DEFINITIONS "__FILENAME__=\"${basename}\"")
    endforeach()
endfunction()

project(
    zero
    VERSION 0.0.1
    LANGUAGES C CXX ASM
    DESCRIPTION "RP2040 Zero")

if(DEFINED ENV{CONTROL_PROD})
    # In a production build, install the libraries to the system
    message("Production Build")
    set(CONTROL_LIB_DIR "lib")
    set(CONTROL_BIN_DIR "bin")
else()
    # In a development build, keep the libraries and binaries local
    message("Development Build")
    set(CONTROL_LIB_DIR "$ENV{PWD}/lib")
    set(CONTROL_BIN_DIR "$ENV{PWD}/bin")
endif()

if(DEFINED ENV{CONTROL_PROJECT})
    set( CONTROL_PROJECT "$ENV{CONTROL_PROJECT}" )
else()
    message("No project set, building example project")
    set( CONTROL_PROJECT "oled" )
endif()
message("Building ${CONTROL_PROJECT}")

# Initialize the pico sdk
pico_sdk_init()

add_subdirectory(app)
add_subdirectory(common)
add_subdirectory(project/${CONTROL_PROJECT})
