#include "core/draw/canvas.h"

static const uint8_t canvas_reverse_nibble_lut[] = {
    0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE, 
    0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF
};

static bool debug = false;

void canvas_debug(bool enable)
{
    debug = enable;
}

void canvas_initialize(Canvas *canvas, uint32_t height, uint32_t width)
{
    // Width needs to be reduced to bytes
    uint32_t image_height_bytes = height;
    uint32_t image_width_bytes = (width % 8 == 0) ? (width / 8) : ((width / 8) + 1);
    canvas->image = (uint8_t*)malloc((image_height_bytes * image_width_bytes) + 1);
    
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
        printf("%02d: ", y);
        for(x = 0; x < (canvas->width / 8); x++) {
            bmpss_print_pixel(canvas->image[x + (y * (canvas->width / 8))]);
        }
        printf("\n");
    }
}

void canvas_set_pixel(Canvas *canvas, uint32_t x_point, uint32_t y_point, uint32_t rotate, uint32_t mirror, CanvasColor color)
{
    if(x_point >= canvas->width) {
        return;
    } else if(y_point >= canvas->height) {
        return;
    }

    uint32_t x = x_point;
    uint32_t y = y_point;    

    switch(rotate) {
    case 0:
        x = x_point;
        y = y_point;  
        break;
    case 90:
        x = y_point;
        y = canvas->height - x_point - 1;
        break;
    case 180:
        x = canvas->width - x_point - 1;
        y = canvas->height - y_point - 1;
        break;
    case 270:
        x = canvas->width - y_point - 1;
        y = x_point;
        break;
    default:
        return;
    }

    switch(mirror) {
    case CANVAS_MIRROR_NONE:
        break;
    case CANVAS_MIRROR_HORIZONTAL:
        x = canvas->width - x - 1;
        break;
    case CANVAS_MIRROR_VERTICAL:
        y = canvas->height - y - 1;
        break;
    default:
        break;
    }

    uint32_t canvas_x = x / 8;
    uint32_t canvas_y = y * (canvas->width / 8);
    if(color == CC_BLACK) {
        canvas->image[canvas_x + canvas_y] &= ~(0x1 << (/*7 - */(x % 8)));
    } else {
        canvas->image[canvas_x + canvas_y] |= (0x1 << (/*7 - */(x % 8)));
    }
}

void canvas_draw_point(Canvas *canvas, uint32_t x_point, uint32_t y_point, 
                       CanvasColor color, uint8_t size)
{
    uint32_t x_coordinate = 0;
    uint32_t y_coordinate = 0;
    for(x_coordinate = 0; x_coordinate < size; x_coordinate++) {
        for(y_coordinate = 0; y_coordinate < size; y_coordinate++) {
            canvas_set_pixel(canvas, (x_point + x_coordinate), 
                             (y_point + y_coordinate), canvas->rotate, canvas->mirror, color);
        }
    }
}

void canvas_draw_line(Canvas *canvas, uint32_t x_start, uint32_t y_start, 
                      uint32_t x_end, uint32_t y_end)
{
    int dx = (int)x_end - (int)x_start >= 0 ? x_end - x_start : x_start - x_end;
    int dy = (int)y_end - (int)y_start <= 0 ? y_end - y_start : y_start - y_end;
}

void canvas_draw_bmp_sprite(Canvas *canvas, Bitmap *bmp, bmp_sprite_view *sprite,
                            uint32_t offset_x, uint32_t offset_y)
{
    uint8_t size = sprite->magnify;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t scanline_width = bmpss_scanline_width(bmp);
    
    CanvasColor black = CC_BLACK;
    CanvasColor white = CC_WHITE;
    if(sprite->invert) {
        black = CC_WHITE;
        white = CC_BLACK;
    }

    for(y = 0; y < sprite->height; y++) {
        if(debug){ printf("\n%02d: ", y); }
        for(x = 0; x < (sprite->width / 8); x++) {
            // Need to correlate the bitmap with the canvas. Bitmaps are stored 
            // bottom to top
            uint32_t bmp_x = (sprite->x / 8) + x;
            // Handle from bottom to top
            uint32_t bmp_y = (sprite->y + y) * scanline_width;
            // // Handle from top to bottom
            // uint32_t bmp_y = (((sprite->height + sprite->y) - 1) - y) * scanline_width;

            // canvas->image[canvas_x + canvas_y] = bmp->pixel_data[bmp_x + bmp_y];
            for(uint32_t i = 0; i < 8; i++) {
                
                // Calculate the point on the canvas
                uint32_t canvas_x_point = ((x * 8 * size)) + (i * size);
                uint32_t canvas_y_point = (y * size);

                // If we are rotating the sprite, calculate the rotation
                uint32_t x_point = 0;
                uint32_t y_point = 0;
                switch(sprite->rotate) {
                case(CANVAS_ROTATE_270):
                    x_point = (sprite->width * size - canvas_y_point) - size;
                    y_point = canvas_x_point;
                    break;
                case(CANVAS_ROTATE_180):
                    x_point = (sprite->width * size - canvas_x_point) - size;
                    y_point = (sprite->height * size - canvas_y_point) - size;
                    break;
                case(CANVAS_ROTATE_90):
                    x_point = canvas_y_point;
                    y_point = (sprite->height * size - canvas_x_point) - size;
                    break;
                default:
                    x_point = canvas_x_point;
                    y_point = canvas_y_point;
                    break;
                }

                // Add the offset into the calculated points
                x_point += offset_x;
                y_point += offset_y;

                // printf("%d: canvas_x: %d, canvas_y: %d\n", x, x_point, y_point);
                // Draw the bitmap pixel to the canvas
                if(bmp->pixel_data[bmp_x + bmp_y] & (0x01 << (7 - i))) {
                    if(debug){ printf("-"); }
                    canvas_draw_point(canvas, x_point, y_point, white, size);
                } else {
                    if(debug){ printf("0"); }
                    canvas_draw_point(canvas, x_point, y_point, black, size);
                }
            }
        }
    }
}

void canvas_draw_grayscale_bmp_sprite(Canvas *canvas, Bitmap *bmp, bmp_sprite_view *sprite, uint32_t layer,
                            uint32_t offset_x, uint32_t offset_y)
{
    uint8_t size = sprite->magnify;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t scanline_width = bmpss_scanline_width(bmp);
    
    CanvasColor black = CC_BLACK;
    CanvasColor white = CC_WHITE;
    if(sprite->invert) {
        black = CC_WHITE;
        white = CC_BLACK;
    }

    for(y = 0; y < sprite->height; y++) {
        // printf("\n%02d: ", y);
        for(x = 0; x < (sprite->width / 2); x++) {
            // Need to correlate the bitmap with the canvas. Bitmaps are stored 
            // bottom to top
            uint32_t bmp_x = (sprite->x / 2) + x;
            // Handle from bottom to top
            uint32_t bmp_y = (sprite->y + y) * scanline_width;

            // bmpss_print_grayscale_pixel(bmp->pixel_data[bmp_x + bmp_y]);

            for(uint32_t i = 0; i < 2; i++) {
                uint8_t byte = 0;
                if(i == 0) {
                    byte = bmp->pixel_data[bmp_x + bmp_y] >> 4;
                } else {
                    byte = bmp->pixel_data[bmp_x + bmp_y] & 0xF;
                }
                
                // Calculate the point on the canvas
                uint32_t canvas_x_point = ((x * 2 * size)) + (i * size);
                uint32_t canvas_y_point = (y * size);

                // If we are rotating the sprite, calculate the rotation
                uint32_t x_point = canvas_x_point;
                uint32_t y_point = canvas_y_point;

                switch(sprite->rotate) {
                case(CANVAS_ROTATE_270):
                    x_point = (sprite->width * size - canvas_y_point) - size;
                    y_point = canvas_x_point;
                    break;
                case(CANVAS_ROTATE_180):
                    x_point = (sprite->width * size - canvas_x_point) - size;
                    y_point = (sprite->height * size - canvas_y_point) - size;
                    break;
                case(CANVAS_ROTATE_90):
                    x_point = canvas_y_point;
                    y_point = (sprite->height * size - canvas_x_point) - size;
                    break;
                default:
                    x_point = canvas_x_point;
                    y_point = canvas_y_point;
                    break;
                }

                // Add the offset into the calculated points
                x_point += offset_x;
                y_point += offset_y;

                if(byte == 2) {
                    byte = 1;
                } else if(byte == 1){
                    byte = 2;
                }
                // printf("%d: canvas_x: %d, canvas_y: %d\n", x, x_point, y_point);
                // Draw the bitmap pixel to the canvas
                if(byte > layer) {
                    // printf("-");
                    canvas_draw_point(canvas, x_point, y_point, white, size);
                } else {
                    // printf("0");
                    canvas_draw_point(canvas, x_point, y_point, black, size);
                }
            }
        }
    }
    // printf("\n");
}
