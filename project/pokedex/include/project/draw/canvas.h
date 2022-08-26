#ifndef DRAW_CANVAS_H
#define DRAW_CANVAS_H

#include <stdint.h>
#include <stdlib.h>

#include "project/bmpspritesheet.h"

typedef struct {
    uint8_t *image;
    uint32_t height;
    uint32_t width;
} Canvas;

void canvas_initialize(Canvas *canvas, uint32_t height, uint32_t width);
void canvas_deinitialize(Canvas *canvas);

void canvas_fill(Canvas *canvas, uint8_t byte);

void canvas_print(Canvas *canvas);


void canvas_draw_line(Canvas *canvas, uint32_t x_start, uint32_t y_start, 
                      uint32_t x_end, uint32_t y_end);

void canvas_draw_bmp_sprite(Canvas *canvas, Bitmap *bmp, BmpSprite *sprite,
                            uint32_t offset_x, uint32_t offset_y);

#endif // DRAW_CANVAS_H