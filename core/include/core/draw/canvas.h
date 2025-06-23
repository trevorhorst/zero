#ifndef DRAW_CANVAS_H
#define DRAW_CANVAS_H

#include <stdint.h>
#include <stdlib.h>

#include "core/draw/bmpspritesheet.h"

typedef struct {
    uint8_t *image;
    uint32_t size;
    uint32_t height;
    uint32_t width;
    uint32_t rotate;
    uint32_t mirror;
} Canvas;

typedef enum {
    CC_BLACK   = 0,
    CC_WHITE   = 1
} CanvasColor;

typedef enum {
    PIXEL_1X1 = 1,
    PIXEL_2X2 = 2,
    PIXEL_3X3 = 3,
    PIXEL_4X4 = 4,
} CanvasPointSize;

#define CANVAS_MIRROR_NONE          0
#define CANVAS_MIRROR_VERTICAL      1
#define CANVAS_MIRROR_HORIZONTAL    2

#define CANVAS_ROTATE_0     0
#define CANVAS_ROTATE_90    90
#define CANVAS_ROTATE_180   180
#define CANVAS_ROTATE_270   270

void canvas_debug(bool enable);

void canvas_initialize(Canvas *canvas, uint32_t height, uint32_t width);
void canvas_deinitialize(Canvas *canvas);

void canvas_fill(Canvas *canvas, uint8_t byte);

void canvas_print(Canvas *canvas);


void canvas_set_pixel(Canvas *canvas, uint32_t x_point, uint32_t y_point, uint32_t rotate, uint32_t mirror,
                      CanvasColor color);

void canvas_draw_point(Canvas *canvas, uint32_t x_point, uint32_t y_point,
                       CanvasColor color, uint8_t size);

void canvas_draw_line(Canvas *canvas, uint32_t x_start, uint32_t y_start,
                      uint32_t x_end, uint32_t y_end, CanvasColor color);

void canvas_draw_rectangle(Canvas *canvas, uint32_t Xstart, uint32_t Ystart, uint32_t Xend, uint32_t Yend,
                         CanvasColor color /*, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill*/);

void canvas_draw_circle(Canvas *canvas, uint32_t x_center, uint32_t y_center, uint32_t radius,
                      CanvasColor color /*, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill*/);

void canvas_draw_bmp_sprite(Canvas *canvas, Bitmap *bmp, bmp_sprite_view *sprite,
                            uint32_t offset_x, uint32_t offset_y);

void canvas_draw_grayscale_bmp_sprite(Canvas *canvas, Bitmap *bmp, bmp_sprite_view *sprite, uint32_t layer,
                            uint32_t offset_x, uint32_t offset_y);

#endif // DRAW_CANVAS_H
