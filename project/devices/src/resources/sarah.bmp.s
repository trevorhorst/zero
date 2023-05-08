    .section .rodata
    .global sarah_bmp
    .type   sarah_bmp, %object
    .align  4
sarah_bmp:
    .incbin "sarah.bmp"
sarah_bmp_end:
    .global sarah_bmp_size
    .type   sarah_bmp_size, %object
    .align  4
sarah_bmp_size:
    .int    sarah_bmp_end - sarah_bmp