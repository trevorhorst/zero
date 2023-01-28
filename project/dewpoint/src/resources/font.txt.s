    .section .rodata
    .global font_txt
    .type   font_txt, %object
    .align  4
font_txt:
    .incbin "font.txt"
font_txt_end:
    .global font_txt_size
    .type   font_txt_size, %object
    .align  4
font_txt_size:
    .int    font_txt_end - font_txt
