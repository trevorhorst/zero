cmake_minimum_required(VERSION 3.5)
project(bitmapreader
    VERSION 
        0.0.1
    DESCRIPTION
        "Bitmap image reader"
    LANGUAGES 
        ASM C
    )

add_executable(
    ${PROJECT_NAME}
        main.c
        bmp.h
        bmp.c
)
