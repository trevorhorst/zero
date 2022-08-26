cmake_minimum_required(VERSION 3.13)
project(
    appd
    DESCRIPTION "Application"
    VERSION 1.0.0
    LANGUAGES C CXX ASM
)
# enable_language( C ASM )

add_executable(${PROJECT_NAME} main.cpp)

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        ${CONTROL_PROJECT}
    )   

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(
    ${PROJECT_NAME}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CONTROL_BIN_DIR})
