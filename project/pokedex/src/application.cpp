#include "pico/multicore.h"

#include "common/drivers/epaper.h"
#include "common/drivers/ws2812.h"

#include "project/application.h"
#include "project/bmpspritesheet.h"
#include "project/draw/draw.h"
#include "project/draw/canvas.h"
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

WS2812 neopixel(PIN_NEOPIXEL, NEOPIXEL_NUM_LEDS, pio0, 0, WS2812::DataFormat::FORMAT_GRB);

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

    BmpSprite sprite;
    sprite.height = SPRITE_HEIGHT;
    sprite.width = SPRITE_WIDTH;
    sprite.x = 0;
    sprite.y = 0;

    Canvas canvas;
    canvas_initialize(&canvas, EPD_1IN54_V2_HEIGHT, EPD_1IN54_V2_WIDTH);
    canvas_fill(&canvas, 0xAA);
    printf("Black out the screen\n");

    int32_t dexNumber = 0;
    multicore_launch_core1(heartbeat);
    
    int32_t offset_x = ((EPD_1IN54_V2_HEIGHT - SPRITE_HEIGHT) / 2);
    int32_t offset_y = ((EPD_1IN54_V2_WIDTH - SPRITE_WIDTH) / 2);
    
    // Update the pokemon every second
    while(true) {
        dexNumber++;
        printf("Pokedex Number: %d\n", dexNumber);

        if(dexNumber >= 1 && dexNumber <= 151) {
            // Calculate the x, y coordinates of our pokemon sprite
            int32_t sheetWidth = ss.bitmap.info_header.width / SPRITE_WIDTH;
            int32_t sheetHeight = ss.bitmap.info_header.height / SPRITE_HEIGHT;
            int32_t x = (sheetWidth - ((160 - dexNumber) % sheetWidth)) - 1;
            int32_t y = (160 - dexNumber) / sheetWidth;
            sprite.x = SPRITE_WIDTH * (x);
            sprite.y = SPRITE_HEIGHT * (y);
            printf("(%d, %d)\n", sprite.x, sprite.y);
        } else {
            dexNumber = 0;
        } 

        canvas_draw_bmp_sprite(&canvas, &(ss.bitmap), &sprite, offset_x, offset_y);

        paper.reset();
        paper.display(canvas.image);
        paper.sleep();

        canvas_fill(&canvas, 0xAA);
        sleep_ms(5000);
    }

    return success;
}