#ifndef BMPSPRITESHEET_H
#define BMPSPRITESHEET_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    uint8_t signature[2];
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
} __attribute__((packed)) BitmapHeader;

typedef struct {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
    uint32_t imageSize;
    uint32_t xPixelsPerM;
    uint32_t yPixelsPerM;
    uint32_t colorsUsed;
    uint32_t importantColors;
} __attribute__((packed)) BitmapInfoHeader;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t reserved;
} __attribute__((packed)) ColorTable;

typedef struct {
    BitmapHeader header;
    BitmapInfoHeader info_header;
    ColorTable *color_table;
    uint8_t *pixel_data;
} Bitmap;

typedef struct {
    char *filename;
    Bitmap bitmap;
} BmpSpriteSheet;

typedef struct {
    int32_t height;
    int32_t width;
    int32_t x;
    int32_t y;
    int8_t invert;
    int8_t magnify;
} BmpSprite;

/**
 * @brief Initialize the sprite sheet struct 
 * 
 * @param ss Pointer to a sprite sheet object
 * @param filename Name of the file used to initialize the sprite sheet object
 * @return int8_t Value indicating success of the operation (0 on success)
 */
int8_t bmpss_initialize(BmpSpriteSheet *ss, const char *filename);

int8_t bmpss_initialize(BmpSpriteSheet *ss, char *resource, unsigned int resource_size);

/**
 * @brief De-Initialize the sprite sheet struct
 * 
 * @param ss Pointer to a sprite sheet object
 * @return int8_t Value indicating success of the operation (0 on success)
 */
int8_t bmpss_deinitialize(BmpSpriteSheet *ss);

int32_t bmpss_scanline_width(Bitmap *bmp);

/**
 * @brief  Prints the entire sprite sheet to console for easy viewing
 * 
 * @param ss Pointer to a sprite sheet object
 */
void bmpss_print_sheet(BmpSpriteSheet *ss);

/**
 * @brief  Prints a subset of the sprite sheet for easy viewing 
 * 
 * @param ss Pointer to a sprite sheet object
 * @param sprite Pointer to a sprite object
 */
void bmpss_print_sprite(BmpSpriteSheet *ss, BmpSprite *sprite);

#endif // BMPSPRITESHEET_H