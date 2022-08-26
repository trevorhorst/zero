#include "project/draw/canvas.h"

void canvas_initialize(Canvas *canvas, uint32_t height, uint32_t width)
{
    // Width needs to be reduced to bytes
    uint32_t image_height_bytes = height;
    uint32_t image_width_bytes = (width % 8 == 0) ? (width / 8) : ((width / 8) + 1);
    canvas->image = (uint8_t*)malloc(image_height_bytes * image_width_bytes);
    
    if(canvas->image == NULL) {
        canvas->height = 0;
        canvas->width  = 0;
    } else {
        // Memory successfully acquired, initialized the canvas
        canvas->height = height;
        canvas->width  = width;
        canvas_fill(canvas, 0xFF);
    }
}

void canvas_deinitialize(Canvas *canvas)
{
    free(canvas->image);
    canvas->height = 0;
    canvas->width = 0;
}

void canvas_fill(Canvas *canvas, uint8_t byte)
{
    int32_t x = 0;
    int32_t y = 0;
    for(y = 0; y < canvas->height; y++) {
        for(x = 0; x < (canvas->width / 8); x++) {
            canvas->image[x + (y * (canvas->width / 8))] = byte;
        }
    }
}

void canvas_print(Canvas *canvas)
{
    int32_t x = 0;
    int32_t y = 0;
    for(y = 0; y < canvas->height; y++) {
        for(x = 0; x < (canvas->width / 8); x++) {
            printf("%02X ", canvas->image[x + (y * (canvas->width / 8))]);
        }
        printf("\n");
    }
}

void canvas_draw_line(Canvas *canvas, uint32_t x_start, uint32_t y_start, 
                      uint32_t x_end, uint32_t y_end)
{
    int dx = (int)x_end - (int)x_start >= 0 ? x_end - x_start : x_start - x_end;
    int dy = (int)y_end - (int)y_start <= 0 ? y_end - y_start : y_start - y_end;
}


void canvas_draw_bmp_sprite(Canvas *canvas, Bitmap *bmp, BmpSprite *sprite,
                            uint32_t offset_x, uint32_t offset_y)
{
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t scanline_width = bmpss_scanline_width(bmp);
    for(y = 0; y < sprite->height; y++) {
        for(x = 0; x < (sprite->width / 8); x++) {
            // Need to correlate the bitmap with the image. Bitmaps are stored 
            // bottom to top
            uint32_t bmp_x = (sprite->x / 8) + x;
            uint32_t bmp_y = (((sprite->height + sprite->y) - 1) - y) * scanline_width;
            uint32_t canvas_x = x + (offset_x / 8);
            uint32_t canvas_y = (y + offset_y) * (canvas->width / 8);
            canvas->image[canvas_x + canvas_y] = bmp->pixel_data[bmp_x + bmp_y];
        }
    }
}