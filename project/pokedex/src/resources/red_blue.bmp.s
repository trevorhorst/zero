    .section .rodata
    .global red_blue_bmp
    .type   red_blue_bmp, %object
    .align  4
red_blue_bmp:
    .incbin "red_blue.bmp"
red_blue_bmp_end:
    .global red_blue_bmp_size
    .type   red_blue_bmp_size, %object
    .align  4
red_blue_bmp_size:
    .int    red_blue_bmp_end - red_blue_bmp
