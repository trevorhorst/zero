#include "common/draw/canvas.h"

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
        for(x = 0; x < (canvas->width / 8); x++) {
            bmpss_print_pixel(canvas->image[x + (y * (canvas->width / 8))]);
            // printf("%02X ", canvas->image[x + (y * (canvas->width / 8))]);
        }
        printf("\n");
    }
}

void canvas_set_pixel(Canvas *canvas, uint32_t x_point, uint32_t y_point, uint32_t rotate, uint32_t mirror, CanvasColor color)
{
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
    if(color == CanvasColor::BLACK) {
        canvas->image[canvas_x + canvas_y] &= ~(0x1 << (/*7 - */(x % 8)));
    } else {
        canvas->image[canvas_x + canvas_y] |= (0x1 << (/*7 - */(x % 8)));
    }
}

void canvas_set_sprite_pixel(Canvas *canvas, BmpSprite *sprite, uint32_t x_point, uint32_t y_point, uint32_t rotate, uint32_t mirror, CanvasColor color)
{
    uint32_t x = x_point;
    uint32_t y = y_point;    

    // Rotates in degress to the right
    switch(rotate) {
    case 0:
        x = x_point;
        y = y_point;  
        break;
    case 90:
        x = y_point;
        y = sprite->height - x_point - 1;
        break;
    case 180:
        x = sprite->width - x_point - 1;
        y = sprite->height - y_point - 1;
        break;
    case 270:
        x = sprite->width - y_point - 1;
        y = x_point;
        break;
    default:
        return;
    }

    switch(mirror) {
    case CANVAS_MIRROR_NONE:
        break;
    case CANVAS_MIRROR_HORIZONTAL:
        x = sprite->width - x - 1;
        break;
    case CANVAS_MIRROR_VERTICAL:
        y = sprite->height - y - 1;
        break;
    default:
        break;
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
            canvas_set_pixel(canvas, (x_point + x_coordinate), 
                             (y_point + y_coordinate), canvas->rotate, canvas->mirror, color);
        }
    }
}

void canvas_draw_sprite_point(Canvas *canvas, BmpSprite *sprite, uint32_t x_point, uint32_t y_point, 
                       CanvasColor color, uint8_t size)
{
    uint32_t x_coordinate = 0;
    uint32_t y_coordinate = 0;
    for(x_coordinate = 0; x_coordinate < size; x_coordinate++) {
        for(y_coordinate = 0; y_coordinate < size; y_coordinate++) {
            canvas_set_sprite_pixel(canvas, sprite, (x_point + x_coordinate), 
                             (y_point + y_coordinate), 0, CANVAS_MIRROR_NONE, color);
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
        if(debug){ printf("\n%02d: ", y); }
        for(x = 0; x < (sprite->width / 8); x++) {
            // Need to correlate the bitmap with the canvas. Bitmaps are stored 
            // bottom to top
            uint32_t bmp_x = (sprite->x / 8) + x;
            uint32_t bmp_y = (((sprite->height + sprite->y) - 1) - y) * scanline_width;
            // uint32_t canvas_x = x + (offset_x / 8);
            // uint32_t canvas_y = (y + offset_y) * (canvas->width / 8);

            // canvas->image[canvas_x + canvas_y] = bmp->pixel_data[bmp_x + bmp_y];
            for(uint32_t i = 0; i < 8; i++) {
                uint32_t canvas_x_point = ((x * 8 * size) + offset_x) + (i * size);
                uint32_t canvas_y_point = offset_y + (y * size);
                // printf("%d: canvas_x: %d, canvas_y: %d\n", x, canvas_x_point, canvas_y_point);
                if(bmp->pixel_data[bmp_x + bmp_y] & (0x01 << (7 - i))) {
                    if(debug){ printf("-"); }
                    canvas_draw_point(canvas, canvas_x_point, canvas_y_point, white, size);
                } else {
                    if(debug){ printf("0"); }
                    canvas_draw_point(canvas, canvas_x_point, canvas_y_point, black, size);
                }
            }
        }
    }
}

void canvas_byte_flip(Canvas *canvas)
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t bit = 0;
    for(y = 0; y < canvas->height; y++) {
        for(x = 0; x < (canvas->width / 8); x++) {
            uint8_t byte = canvas->image[x + (y * (canvas->width / 8))];
            canvas->image[x + (y * (canvas->width / 8))] = 
                (canvas_reverse_nibble_lut[byte & 0xF] << 4) |
                (canvas_reverse_nibble_lut[byte >> 4]);
            // for(bit = 0; bit < 8; bit++) {
            //     if(byte & (0x1 << bit)) {
            //         canvas->image[x + (y * (canvas->width / 8))] |= (0x80 >> bit);
            //     }
            // }
        }
    }
}