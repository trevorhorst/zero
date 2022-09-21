#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "bmpspritesheet.h"

static const char *bmp_file = "red-blue-grayscale.bmp";

#define SPRITE_WIDTH    56
#define SPRITE_HEIGHT   56

int main(int argc, char *argv[])
{
    int32_t dexNumber = -1;
    if(argc == 2) {
        dexNumber = atoi(argv[1]);
    }

    // Initialize the sprite sheet from file
    BmpSpriteSheet ss;
    bmpss_initialize(&ss, bmp_file);

    BmpSprite sprite;
    sprite.height = SPRITE_HEIGHT;
    sprite.width = SPRITE_WIDTH;
    sprite.x = 0;
    sprite.y = 0;

    if(dexNumber >= 1 && dexNumber <= 151) {
        // Calculate the x, y coordinates of our pokemon sprite
        int32_t sheetWidth = ss.bitmap.info_header.width / SPRITE_WIDTH;
        int32_t sheetHeight = ss.bitmap.info_header.height / SPRITE_HEIGHT;
        int32_t sheetSprites = sheetWidth * sheetHeight;
        int32_t x = (sheetWidth - ((sheetSprites - dexNumber) % sheetWidth)) - 1;
        int32_t y = (sheetSprites - dexNumber) / sheetWidth;
        sprite.x = SPRITE_WIDTH * (x);
        sprite.y = SPRITE_HEIGHT * (y);
        printf("(%d, %d)\n", sprite.x, sprite.y);
    }

    // // Print the desired sprite
    bmpss_print_grayscale_sprite(&ss, &sprite);
    // bmpss_print_raw_data(ss.bitmap.pixel_data, (ss.bitmap.header.fileSize - ss.bitmap.header.dataOffset), 8);
    // pss_print_grayscale_sheet(&ss);
    
    // De-initialize the sprite sheet
    bmpss_deinitialize(&ss);

    return 0;
}
