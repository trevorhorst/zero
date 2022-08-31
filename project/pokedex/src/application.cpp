#include "pico/multicore.h"

#include "common/drivers/epaper.h"
#include "common/drivers/ws2812.h"

#include "project/application.h"
#include "project/bmpspritesheet.h"
#include "project/draw/canvas.h"
#include "project/pokedex.h"
#include "project/resources.h"

#define PIN_DISPLAY_CD      0    // DC
#define PIN_SPI0_CS         1    // CS
#define PIN_SPI0_SCK        2    // CLK
#define PIN_SPI0_MOSI       3    // DIN
#define PIN_DISPLAY_BUSY    4    // BUSY
#define PIN_DISPLAY_RESET   5    // RST
#define PIN_NEOPIXEL        16

#define NEOPIXEL_NUM_LEDS       1
#define NEOPIXEL_MAX_BRIGHTNESS 25

#define UNIT_MHZ(x) x * 1000000

#define SPRITE_WIDTH    56
#define SPRITE_HEIGHT   56

#define SPRITE_FONT_WIDTH    8
#define SPRITE_FONT_HEIGHT   8

#define STR_POKEMON         "%-10s %03d"
#define CHAR_TO_SPRITE(x)   (x - 'A') + 1
#define CHAR_TO_UPPER(x)    (x >= 'a' && x <= 'z') ? x - 0x20 : x

WS2812 neopixel(PIN_NEOPIXEL, NEOPIXEL_NUM_LEDS, pio0, 0, WS2812::DataFormat::FORMAT_GRB);

void index_to_sprite(uint32_t index, BmpSpriteSheet *ss, BmpSprite *sprite)
{
    // Calculate the x, y coordinates of our pokemon sprite
    int32_t sheetWidth = ss->bitmap.info_header.width / sprite->width;
    int32_t sheetHeight = ss->bitmap.info_header.height / sprite->height;
    int32_t sheetSprites = sheetWidth * sheetHeight;
    int32_t x = (sheetWidth - ((sheetSprites - index) % sheetWidth)) - 1;
    int32_t y = (sheetSprites - index) / sheetWidth;
    sprite->x = sprite->width * x;
    sprite->y = sprite->height * y;
    // printf("sprite %d: (%d, %d)\n", index, sprite->x, sprite->y);
}

char index_to_char(uint32_t index)
{
    char s = 128; // SPACE sprite
    if(index >= 'A' && index <= 'Z') {
        s = (index - 'A') + 1;
    } else if(index >= 'a' && index <= 'z') {
        s = (index - 'A') + 1;
    } else if(index >= '0' && index <= '9') {
        s = (index + 38) + 1;
    } else if(index == '\'') {
        s = 65;
    } else if(index == '-') {
        s = 68;
    } else if(index == '?') {
        s = 71;
    } else if(index == '!') {
        s = 72;
    } else if(index == '.') {
        s = 73;
    } else if(index == 0x01) {
        // Male symbol
        s = 80;
    } else if(index == 0x02) {
        // Female symbol
        s = 86;
    } else if(index == 0x20) {
        // Space
        s = 128;
    } else if(index == 0x2C) {
        s = 85;
    } else if(index == 0x82) {
        // Fancy e 
        s = 59;
    } else {
        printf("Missing character 0x%02X\n", (uint8_t)index);
    }
    return s;
}

void heartbeat()
{
    // Just run a heartbeat LED to indicate we are running
    int32_t green_level = 0;
    int32_t modifier = 1;
    while(true) {
        green_level += modifier ;
        neopixel.fill(WS2812::RGB(0, green_level, 0));
        neopixel.show();
        if(green_level == NEOPIXEL_MAX_BRIGHTNESS) {
            modifier = -1;
        } else if(green_level == 0) {
            modifier = 1;
        }
        sleep_ms(100);
    }
}

int32_t application_run()
{
    int32_t success = 0;

    // Standard RP2040 initialization
    stdio_init_all();

    sleep_ms(1000);

    // Initialize GPIO
    gpio_set_function(PIN_SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI0_MOSI, GPIO_FUNC_SPI);

    // Configure pin 0 for command/data on the display
    gpio_init(PIN_DISPLAY_CD);
    gpio_set_dir(PIN_DISPLAY_CD, GPIO_OUT);
    gpio_put(PIN_DISPLAY_CD, 0);

    // Configure pin 1 for chip select on the SPI
    gpio_init(PIN_SPI0_CS);
    gpio_set_dir(PIN_SPI0_CS, GPIO_OUT);
    gpio_put(PIN_SPI0_CS, 0);

    // Configure pin 4 for the busy line on the display
    gpio_init(PIN_DISPLAY_BUSY);
    gpio_set_dir(PIN_DISPLAY_BUSY, GPIO_IN);

    // Configure pin 5 for chip select on the SPI
    gpio_init(PIN_DISPLAY_RESET);
    gpio_set_dir(PIN_DISPLAY_RESET, GPIO_OUT);
    gpio_put(PIN_DISPLAY_RESET, 1);

    // Initialize the SPI port to 
    spi_init(spi0, UNIT_MHZ(10));
    spi_set_format(
        spi0,
        8,
        SPI_CPOL_1,
        SPI_CPHA_1,
        SPI_MSB_FIRST
    );

    // Initialize the boards neopixel
    neopixel.fill(WS2812::RGB(1, 0, 0));
    neopixel.show();

    // Initialize epaper driver and a blank image buffer
    DrvEPaper paper(spi0, PIN_SPI0_CS, PIN_DISPLAY_CD, PIN_DISPLAY_BUSY, PIN_DISPLAY_RESET);
    paper.initialize();
    paper.fillScreen(0xFF);

    // Initialize the sprite sheet we will be using to draw bitmaps
    BmpSpriteSheet ss;
    bmpss_initialize(&ss, red_blue_bmp, red_blue_bmp_size);

    BmpSpriteSheet ss_font;
    bmpss_initialize(&ss_font, red_blue_font_bmp, red_blue_font_bmp_size);

    BmpSprite sprite;
    sprite.height = SPRITE_HEIGHT;
    sprite.width = SPRITE_WIDTH;
    sprite.x = 0;
    sprite.y = 0;
    sprite.invert = 0;

    Canvas canvas;
    canvas_initialize(&canvas, EPD_1IN54_V2_HEIGHT, EPD_1IN54_V2_WIDTH);
    canvas_fill(&canvas, 0xFF);
    // canvas_draw_point(&canvas, 20, 20, CanvasColor::BLACK, CanvasPointSize::PIXEL_4X4);
    printf("Black out the screen\n");
    paper.display(canvas.image);
    paper.sleep();

    // sleep_ms(5000);

    int32_t dexNumber = 0;
    int32_t dexChar   = 0;
    multicore_launch_core1(heartbeat);
    

    uint32_t poke_sprite_magnify = 2;
    uint32_t offset_x = ((EPD_1IN54_V2_HEIGHT - (SPRITE_HEIGHT * poke_sprite_magnify)) / 2);
    uint32_t offset_y = ((EPD_1IN54_V2_WIDTH - (SPRITE_WIDTH * poke_sprite_magnify)) / 2) - 10;
    
    char pokemon_name[15];
    // Update the pokemon every second
    while(true) {
        dexNumber++;
        printf("Pokedex Number: %d\n", dexNumber);
        snprintf(pokemon_name, sizeof(pokemon_name), STR_POKEMON, pokedex[dexNumber - 1]->name, dexNumber);

        uint32_t i = 0;
        for(i = 0; i < strlen(pokemon_name); i++) {
            dexChar = index_to_char(CHAR_TO_UPPER(pokemon_name[i]));
            if(dexChar >= 0) {
                sprite.height = SPRITE_FONT_HEIGHT;
                sprite.width = SPRITE_FONT_WIDTH;
                sprite.magnify = 1;
                index_to_sprite(dexChar, &ss_font, &sprite);

            }
            uint32_t char_offset_x = (((SPRITE_FONT_WIDTH * i) * sprite.magnify) + 1) + (SPRITE_FONT_HEIGHT * 5);
            uint32_t char_offset_y = 5;
            canvas_draw_bmp_sprite(&canvas, &(ss_font.bitmap), &sprite, char_offset_x, char_offset_y);
        }

        if(dexNumber >= 1 && dexNumber <= POKEDEX_NUM_POKEMON) {
            sprite.height = SPRITE_HEIGHT;
            sprite.width = SPRITE_WIDTH;
            sprite.magnify = poke_sprite_magnify;
            index_to_sprite(dexNumber, &ss, &sprite);
        } else {
            dexNumber = 0;
        } 
        canvas_draw_bmp_sprite(&canvas, &(ss.bitmap), &sprite, offset_x, offset_y);

        for(i = 0; i < strlen(pokedex[dexNumber - 1]->entry); i++) {
            dexChar = index_to_char(pokedex[dexNumber - 1]->entry[i]);
            if(dexChar >= 0) {
                sprite.height = SPRITE_FONT_HEIGHT;
                sprite.width = SPRITE_FONT_WIDTH;
                sprite.magnify = 1;
                index_to_sprite(dexChar, &ss_font, &sprite);

            }
            uint32_t char_offset_x = (((SPRITE_FONT_WIDTH * (i % 25)) * sprite.magnify) + 1);
            uint32_t char_offset_y = 160 + ((i / 25) * 8) + 1;
            canvas_draw_bmp_sprite(&canvas, &(ss_font.bitmap), &sprite, char_offset_x, char_offset_y);
        }

        paper.reset();
        paper.display(canvas.image);
        paper.sleep();

        canvas_fill(&canvas, 0xFF);
        sleep_ms(5000);
    }

    return success;
}