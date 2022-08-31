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

void canvas_set_pixel(Canvas *canvas, uint32_t x_point, uint32_t y_point, uint32_t rotate, CanvasColor color)
{
    uint32_t x = x_point;
    uint32_t y = y_point;    

    switch(rotate) {
    case 0:
        x = x_point;
        y = y_point;  
        break;
    case 90:
        x = canvas->width - y_point - 1;
        y = x_point;
        break;
    case 180:
        x = canvas->width - x_point - 1;
        y = canvas->height - y_point - 1;
        break;
    case 270:
        x = y_point;
        y = canvas->height - x_point - 1;
        break;
    default:
        return;
    }

    uint32_t canvas_x = x / 8;
    uint32_t canvas_y = y * (canvas->width / 8);
    if(color == CanvasColor::BLACK) {
        canvas->image[canvas_x + canvas_y] &= ~(0x80 >> (x % 8));
    } else {
        canvas->image[canvas_x + canvas_y] |= (0x80 >> (x % 8));
    }
}

void canvas_draw_point(Canvas *canvas, uint32_t x_point, uint32_t y_point, 
                       CanvasColor color, uint8_t size)
{
    uint32_t x_coordinate = 0;
    uint32_t y_coordinate = 0;
    for(x_coordinate = 0; x_coordinate < size; x_coordinate++) {
        for(y_coordinate = 0; y_coordinate < size; y_coordinate++) {
            canvas_set_pixel(canvas, (x_point + x_coordinate - 1), 
                             (y_point + y_coordinate - 1), 270, color);
        }
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
    uint8_t size = sprite->magnify;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t scanline_width = bmpss_scanline_width(bmp);
    
    CanvasColor black = CanvasColor::BLACK;
    CanvasColor white = CanvasColor::WHITE;
    if(sprite->invert) {
        black = CanvasColor::WHITE;
        white = CanvasColor::BLACK;
    }

    for(y = 0; y < sprite->height; y++) {
        for(x = 0; x < (sprite->width / 8); x++) {
            // Need to correlate the bitmap with the canvas. Bitmaps are stored 
            // bottom to top
            uint32_t bmp_x = (sprite->x / 8) + x;
            uint32_t bmp_y = (((sprite->height + sprite->y) - 1) - y) * scanline_width;
            uint32_t canvas_x = x + (offset_x / 8);
            uint32_t canvas_y = (y + offset_y) * (canvas->width / 8);

            // canvas->image[canvas_x + canvas_y] = bmp->pixel_data[bmp_x + bmp_y];
            for(uint32_t i = 0; i < 8; i++) {
                uint32_t canvas_x_point = ((x * 8 * size) + offset_x) + (i * size);
                uint32_t canvas_y_point = offset_y + (y * size);
                // printf("%d: canvas_x: %d, canvas_y: %d\n", x, canvas_x_point, canvas_y_point);
                if(bmp->pixel_data[bmp_x + bmp_y] & (0x80 >> i)) {
                    canvas_draw_point(canvas, canvas_x_point, canvas_y_point, white, size);
                } else {
                    canvas_draw_point(canvas, canvas_x_point, canvas_y_point, black, size);
                }
            }
        }
    }
}