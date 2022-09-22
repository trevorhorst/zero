#include "bmp.h"

void bmpss_print_header(const BitmapHeader *header)
{
    printf("-- Header ---------------------------------------------\n");
    printf("     Signature: %c %c\n", header->signature[0], header->signature[1]);
    printf("     File Size: %d bytes\n", header->fileSize);
    printf("   Data Offset: 0x%X\n", header->dataOffset);
}

void bmpss_print_info_header(const BitmapInfoHeader *info)
{
    printf("-- Info Header ----------------------------------------\n");
    printf("          Size: %d bytes\n", info->size);
    printf("         Width: %d\n", info->width);
    printf("        Height: %d\n", info->height);
    printf("        Planes: %d\n", info->planes);
    printf("Bits Per Pixel: %d\n", info->bitsPerPixel);
    printf("   Compression: %d\n", info->compression);
    printf("   Colors Used: 0x%08X\n", info->colorsUsed);
}

void bmpss_print_raw_data(uint8_t *data, uint32_t size, uint32_t width)
{
    printf("-- Raw Data (%d) ----------------------------------------\n", size);
    for(uint32_t i = 0; i < size; i++) {
        if((i % width) == 0) {
            printf("%08x: ", (i/width) * width);
        }
        printf("%02x ", data[i]);
        if(((i+1) != 0) && ((i+1) % width) == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void bmpss_print_pixel(uint8_t byte)
{
    for(int8_t i = 0; i < 8; i++) {
        // uint8_t shift = 1 << (7 - i);
        uint8_t shift = 1 << (i);
        if(byte & shift) {
            printf("0");
        } else {
            printf(" ");
        }
    }
}

void bmpss_print_grayscale_pixel(uint8_t byte)
{
    byte = byte & 0xF;
    printf("%c", ascii_grayscale[byte]);
}

int8_t bmpss_initialize(BmpSpriteSheet *ss, const char *filename)
{
    int8_t success = 0;
    FILE *fp = fopen(filename, "rb");

    if(fp == NULL) {
        success = -1;
    } else {
        // Get the header
        char *header = (char*)&(ss->bitmap.header);
        uint8_t read = 0;
        read = fread(header, sizeof(char), sizeof(BitmapHeader), fp);
        if(read == sizeof(BitmapHeader)) {
            bmpss_print_header(&(ss->bitmap.header));
        }
    
        // Get the infoheader
        header = (char*)&(ss->bitmap.info_header);
        read = fread(header, sizeof(char), sizeof(BitmapInfoHeader), fp);
        if(read == sizeof(BitmapInfoHeader)) {
            bmpss_print_info_header(&(ss->bitmap.info_header));
        }

        // Malloc memory for the color table that is of variable size
        // uint32_t color_table_size = sizeof(ColorTable) * ss->bitmap.info_header.colorsUsed;
        uint32_t color_table_size = ss->bitmap.header.dataOffset - (sizeof(BitmapHeader) + sizeof(BitmapInfoHeader));;
        ColorTable *color_table = (ColorTable*)malloc(color_table_size);
        fread(color_table, sizeof(char), color_table_size, fp);
        ss->bitmap.color_table = color_table;
        bmpss_print_raw_data((uint8_t*)ss->bitmap.color_table, color_table_size, 8);

        // Malloc memory for the pixel data that is of variable size
        uint32_t pixel_data_size = ss->bitmap.header.fileSize - ss->bitmap.header.dataOffset;
        uint8_t *pixel_data = (uint8_t*)malloc(pixel_data_size);
        fread(pixel_data, sizeof(char), pixel_data_size, fp);
        ss->bitmap.pixel_data = pixel_data;
        bmpss_print_raw_data(ss->bitmap.pixel_data, 128, 8);
    
        fclose(fp);
        fp = NULL;
    }

    return success;
}


int8_t bmpss_grayscale_sprite_initialize(BmpSpriteSheet *ss, bmp_grayscale_sprite *sprite)
{
    uint32_t sprite_size = ((sprite->height * (sprite->width)) * ss->bitmap.info_header.bitsPerPixel) / 8;
    sprite->sprite = (uint8_t*)malloc(sprite_size);
    int32_t scanline_width = bmpss_scanline_width(ss);
    for(int32_t h = sprite->height - 1; h >= 0; h--) {
    //for(int32_t h = 0; h < sprite->height; h++) {
        for(int32_t w = 0; w < ((sprite->width) / 2); w++) {
            // int32_t y = ((sprite->height - h - 1) + sprite->y) * scanline_width;
            int32_t y = ((h) + sprite->y) * scanline_width;
            int32_t x = w + (sprite->x / 2);
            int32_t sprite_y = (h * sprite->width);
            int32_t sprite_x = w + (sprite->x / 2);
            sprite->sprite[sprite_y + sprite_x] = ss->bitmap.pixel_data[y + x];
            // printf("bmp (%d, %d) sprite (%d, %d)\n", x, y, sprite_x, sprite_y);
            // bmpss_print_grayscale_pixel(ss->bitmap.pixel_data[y + x]);
        }
        // printf("\n");
    }

    printf("Sprite Size %d\n", sprite_size);
}

int8_t bmpss_deinitialize(BmpSpriteSheet *ss)
{
    int8_t success = 0;

    // Clean up the color table data
    free(ss->bitmap.color_table);
    ss->bitmap.color_table = NULL;

    // Clean up the pixel data
    free(ss->bitmap.pixel_data);
    ss->bitmap.pixel_data = NULL;

    return success;
}

int32_t bmpss_scanline_width(BmpSpriteSheet *ss)
{
    int32_t scanline_width = (((ss->bitmap.info_header.bitsPerPixel * ss->bitmap.info_header.width) + 31) / 32) * 4;
    return scanline_width;
}

void bmpss_print_sheet(BmpSpriteSheet *ss)
{
    int32_t scanline_width = bmpss_scanline_width(ss);
    for(int32_t h = (ss->bitmap.info_header.height - 1); h >= 0; h--) {
        for(int32_t i = 0; i < scanline_width; i++) {
            bmpss_print_pixel(ss->bitmap.pixel_data[(h * scanline_width) + i]);
        }
        printf("\n");
    }
}

void bmpss_print_grayscale_sheet(BmpSpriteSheet *ss)
{
    int32_t scanline_width = bmpss_scanline_width(ss);
    for(int32_t h = (ss->bitmap.info_header.height - 1); h >= 0; h--) {
        for(int32_t i = 0; i < scanline_width; i++) {
            uint8_t pixel = ss->bitmap.pixel_data[(h*scanline_width) + i];
            bmpss_print_grayscale_pixel((pixel >> 4));
            bmpss_print_grayscale_pixel((pixel >> 0));
        }
        printf("\n");
    }
}

void bmpss_print_sprite(BmpSpriteSheet *ss, BmpSprite *sprite)
{
    int32_t scanline_width = bmpss_scanline_width(ss);
    for(int32_t h = 0; h < sprite->height; h++) {
        printf("%02d: ", h);
        for(int32_t w = 0; w < ((sprite->width) / 8); w++) {
            // int32_t y = ((sprite->height - h - 1) + sprite->y) * scanline_width;
            int32_t y = ((h) + sprite->y) * scanline_width;
            int32_t x = w + (sprite->x / 8);
            bmpss_print_pixel(ss->bitmap.pixel_data[y + x]);
        }
        printf("\n");
    }
}

void bmpss_print_grayscale_sprite(BmpSpriteSheet *ss, BmpSprite *sprite)
{
    int32_t scanline_width = bmpss_scanline_width(ss);
    for(int32_t h = sprite->height - 1; h >= 0; h--) {
    //for(int32_t h = 0; h < sprite->height; h++) {
        printf("%02d: ", h);
        for(int32_t w = 0; w < ((sprite->width) / 2); w++) {
            // int32_t y = ((sprite->height - h - 1) + sprite->y) * scanline_width;
            int32_t y = ((h) + sprite->y) * scanline_width;
            int32_t x = w + (sprite->x / 2);
            uint8_t pixel = ss->bitmap.pixel_data[y + x];
            // bmpss_print_grayscale_pixel(ss->bitmap.pixel_data[y + x]);
            bmpss_print_grayscale_pixel((pixel >> 4));
            bmpss_print_grayscale_pixel((pixel >> 0));
        }
        printf("\n");
    }
}


void bmpss_print_grayscale_sprite_(bmp_grayscale_sprite *sprite)
{
    for(int32_t h = sprite->height - 1; h >= 0; h--) {
    //for(int32_t h = 0; h < sprite->height; h++) {
        printf("%02d: ", h);
        for(int32_t w = 0; w < sprite->width; w++) {
            // int32_t y = ((sprite->height - h - 1) + sprite->y) * scanline_width;
            int32_t y = ((h)) * sprite->width;
            int32_t x = (w / 2) + (sprite->x / 2);

            uint8_t pixel =  sprite->sprite[y + x];
            // bmpss_print_grayscale_pixel(sprite->sprite[y + x]);
            // bmpss_print_grayscale_pixel((pixel >> 4));
            // bmpss_print_grayscale_pixel((pixel >> 0));
            uint8_t shamt = (w & 0x1) ? 0 : 4;
            bmpss_print_grayscale_pixel((pixel >> shamt));
        }
        printf("\n");
    }

}

uint8_t bmpss_round_to_closest_pixel(uint8_t pixel)
{
    uint8_t new = 0xFF;
    if(pixel == 0x00 || pixel == 0x2) {
        pixel = 0x00;
    }
}

int8_t bmpss_get_grayscale_sprite_point(bmp_grayscale_sprite *sprite, uint32_t w, uint32_t h)
{
    int32_t y = h * sprite->width;
    int32_t x = (w / 2) + (sprite->x / 2);
    uint8_t shamt = (w & 0x1) ? 0 : 4;
    int8_t point = 0xF & (sprite->sprite[x + y] >> shamt);
    return point;
}

void bmpss_grayscale_dither(bmp_grayscale_sprite *sprite)
{
    // uint32_t sprite_size = sizeof(sprite->sprite);
    // uint8_t *error = (uint8_t*)malloc(sprite_size);
    for(int32_t h = sprite->height - 1; h >= 0; h--) {
        printf("%02d: ", h);
        for(int32_t w = 0; w < sprite->width; w++) {
            int32_t y = ((h)) * sprite->width;
            int32_t x = (w / 2) + (sprite->x / 2);
            uint8_t shamt = (w & 0x1) ? 0 : 4;
            uint8_t old_pixel = 0xF & (sprite->sprite[x + y] >> shamt);
            old_pixel = reverse_2_bit[old_pixel];
            uint8_t new_pixel = (old_pixel <= 0x1) ? 0x00 : 0x3;
            int8_t error = old_pixel - new_pixel;
            
            // Propogate error
            printf("%d ", error);
        }
        printf("\n");
    }


    // free(error);
}