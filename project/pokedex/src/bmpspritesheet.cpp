#include "project/bmpspritesheet.h"

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
        uint8_t shift = 1 << (7 - i);
        if(byte & shift) {
            printf("0");
        } else {
            printf(" ");
        }
    }
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
        uint32_t color_table_size = sizeof(ColorTable) * ss->bitmap.info_header.colorsUsed;
        ColorTable *color_table = (ColorTable*)malloc(color_table_size);
        fread(color_table, sizeof(char), color_table_size, fp);
        ss->bitmap.color_table = color_table;
        // bmpss_print_raw_data((uint8_t*)ss->bitmap.color_table, color_table_size, 8);

        // Malloc memory for the pixel data that is of variable size
        uint32_t pixel_data_size = ss->bitmap.header.fileSize - ss->bitmap.header.dataOffset;
        uint8_t *pixel_data = (uint8_t*)malloc(pixel_data_size);
        fread(pixel_data, sizeof(char), pixel_data_size, fp);
        ss->bitmap.pixel_data = pixel_data;
        // bmpss_print_raw_data(ss->bitmap.pixel_data, pixel_data_size, 8);
    
        fclose(fp);
        fp = NULL;
    }

    return success;
}

int8_t bmpss_initialize(BmpSpriteSheet *ss, char *resource, unsigned int resource_size)
{
    int8_t success = 0;

    uint32_t offset = 0;

    ss->bitmap = *(Bitmap*)&resource[offset];
    bmpss_print_header(&(ss->bitmap.header));
    bmpss_print_info_header(&(ss->bitmap.info_header));
    
    int32_t header_size = sizeof(BitmapHeader) + sizeof(BitmapInfoHeader);
    int32_t color_table_size = sizeof(ColorTable) * ss->bitmap.info_header.colorsUsed;
    int32_t pixel_data_size = resource_size - ss->bitmap.header.dataOffset;

    offset += header_size;
    ss->bitmap.color_table = (ColorTable*)&resource[offset];
    bmpss_print_raw_data((uint8_t*)ss->bitmap.color_table, color_table_size, 8);

    offset += color_table_size;
    ss->bitmap.pixel_data = (uint8_t*)&resource[offset];
    // bmpss_print_raw_data(ss->bitmap.pixel_data, pixel_data_size, 8);

    return success;
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

int32_t bmpss_scanline_width(Bitmap *bitmap)
{
    int32_t scanline_width = (((bitmap->info_header.bitsPerPixel * bitmap->info_header.width) + 31) / 32) * 4;
    return scanline_width;
}

void bmpss_print_sheet(BmpSpriteSheet *ss)
{
    int32_t scanline_width = bmpss_scanline_width(&(ss->bitmap));
    for(int32_t h = ss->bitmap.info_header.height; h >= 0; h--) {
        for(int32_t i = 0; i < scanline_width; i++) {
            bmpss_print_pixel(ss->bitmap.pixel_data[(h * scanline_width) + i]);
        }
        printf("\n");
    }
}

void bmpss_print_sprite(BmpSpriteSheet *ss, BmpSprite *sprite)
{
    int32_t scanline_width = bmpss_scanline_width(&(ss->bitmap));
    for(int32_t h = (sprite->height - 1); h >= 0; h--) {
        for(int32_t w = 0; w < ((sprite->width) / 8); w++) {
            int32_t y = (h + sprite->y) * scanline_width;
            int32_t x = w + (sprite->x / 8);
            bmpss_print_pixel(ss->bitmap.pixel_data[y + x]);
        }
        printf("\n");
    }
}