#include "common/draw/bmpspritesheet.h"
#include "common/draw/canvas.h"
#include "common/drivers/ssd1306.h"
#include "common/logger.h"

#include "project/application.h"
#include "project/resources.h"

// I2C Pins
#define PIN_I2C1_SDA    26
#define PIN_I2C1_SCL    27

// SPI Pins
#define PIN_SPI_MOSI    3
#define PIN_SPI_SCK     2
#define PIN_DISPLAY_RES 26
#define PIN_DISPLAY_DC  27
#define PIN_DISPLAY_CS  28


#define PIN_INCREASE_CLOCK   7
#define PIN_DECREASE_CLOCK   6

#define BUS_SPEED_KHZ(x) x * 1000

#define SSD1306_DISPLAY_ADDR    0x3D

#define SPRITE_WIDTH    56
#define SPRITE_HEIGHT   56

static uint32_t debounce_generate_fact = to_ms_since_boot(get_absolute_time());
static const uint32_t debounce_delay_time = 250;
static int32_t dex_number = 1;
static int8_t trigger_update = 0;

void generate_fact(uint gpio, uint32_t events)
{
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    if(gpio == PIN_INCREASE_CLOCK) {
        if((currentTime - debounce_generate_fact) > debounce_delay_time) {
            // Increase SPI spee
            dex_number++;
            trigger_update = 1;
            if(dex_number < 1) {
                dex_number = 151;
            } else if(dex_number > 151) {
                dex_number = 1;
            }
            debounce_generate_fact = currentTime;
        }
    } else if(gpio == PIN_DECREASE_CLOCK) {
        if((currentTime - debounce_generate_fact) > debounce_delay_time) {
            // Decrease SPI speed
            dex_number--;
            trigger_update = 1;
            if(dex_number < 1) {
                dex_number = 151;
            } else if(dex_number > 151) {
                dex_number = 1;
            }
            debounce_generate_fact = currentTime;
        }
    }
}


void index_to_sprite(uint32_t index, BmpSpriteSheet *ss, bmp_sprite_view *sprite)
{
    // Calculate the x, y coordinates of our pokemon sprite
    int32_t sheetWidth = ss->bitmap.info_header.width / sprite->width;
    int32_t sheetHeight = ss->bitmap.info_header.height / sprite->height;
    int32_t sheetSprites = sheetWidth * sheetHeight;
    int32_t x = (sheetWidth - ((sheetSprites - index) % sheetWidth)) - 1;
    int32_t y = (sheetSprites - index) / sheetWidth;
    sprite->x = sprite->width * x;
    sprite->y = sprite->height * y;
    printf("sprite %d: (%d, %d)\n", index, sprite->x, sprite->y);
}

void initialize_i2c()
{
    LOG_INFO("Initializing I2C...\n");
    i2c_init(i2c1, BUS_SPEED_KHZ(1000));
    gpio_set_function(PIN_I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C1_SDA);
    gpio_pull_up(PIN_I2C1_SCL);
}

void initialize_spi(spi_inst_t *bus)
{
    LOG_INFO("Initialize SPI...\n");
    
    // Configure clk and data pins
    gpio_set_function(PIN_SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_MOSI, GPIO_FUNC_SPI);

    // Initialize the SPI port to 
    spi_init(bus, BUS_SPEED_KHZ(370*4));
    spi_set_format(
        bus,
        8,
        SPI_CPOL_1,
        SPI_CPHA_1,
        SPI_MSB_FIRST
    );
}

int32_t application_run()
{
    int32_t success = 0;

    stdio_init_all();

    sleep_ms(1000);

    LOG_INFO("Initialize GPIO...\n");
    // Initialize IRQ
    gpio_set_irq_enabled_with_callback(PIN_INCREASE_CLOCK, GPIO_IRQ_EDGE_RISE, true, &generate_fact);
    gpio_set_irq_enabled_with_callback(PIN_DECREASE_CLOCK, GPIO_IRQ_EDGE_RISE, true, &generate_fact);

    // Initialize DC pin
    gpio_init(PIN_DISPLAY_DC);
    gpio_set_dir(PIN_DISPLAY_DC, GPIO_OUT);
    gpio_put(PIN_DISPLAY_DC, 0);

    // Initialize chip select pin
    gpio_init(PIN_DISPLAY_CS);
    gpio_set_dir(PIN_DISPLAY_CS, GPIO_OUT);
    gpio_put(PIN_DISPLAY_CS, 0);

    // Initialize reset pin
    gpio_init(PIN_DISPLAY_RES);
    gpio_set_dir(PIN_DISPLAY_RES, GPIO_OUT);
    gpio_put(PIN_DISPLAY_RES, 1);

    // Initialize I2C
    // initialize_i2c();

    // Initialize SPI
    initialize_spi(spi0);

    ssd1306_spi_device display;
    display.bus   = spi0;
    display.cs    = PIN_DISPLAY_CS;
    display.dc    = PIN_DISPLAY_DC;
    display.reset = PIN_DISPLAY_RES;

    LOG_INFO("Initializing Display...\n");
    ssd1306_initialize_device(&display);
    // ssd1306_ignore_ram(&display, true);
    // sleep_ms(500);
    // ssd1306_ignore_ram(&display, false);

    // Initialize our sprite sheets
    BmpSpriteSheet ss;
    bmpss_initialize(&ss, red_blue_bmp, red_blue_bmp_size);

    BmpSpriteSheet ss_grayscale;
    bmpss_initialize(&ss_grayscale, red_blue_grayscale_bmp, red_blue_grayscale_bmp_size);

    BmpSpriteSheet ss_font;
    bmpss_initialize(&ss_font, red_blue_font_bmp, red_blue_font_bmp_size);

    // Initialize our sprites
    bmp_sprite_view sprite;
    sprite.height = SPRITE_HEIGHT;
    sprite.width = SPRITE_WIDTH;
    sprite.invert = 0;
    sprite.magnify = 1;
    sprite.rotate = CANVAS_ROTATE_270;

    bmp_sprite_view font_sprite;
    font_sprite.height = 8;
    font_sprite.width = 8;
    font_sprite.x = 56;
    font_sprite.y = 56;
    font_sprite.invert = 0;
    font_sprite.rotate = CANVAS_ROTATE_270;
    font_sprite.magnify = 1;

    // Since we are using vertical addressing, swap our height and width parameters
    uint32_t image_width_bytes  = OLED_HEIGHT;
    uint32_t image_height_bytes = (OLED_WIDTH % 8 == 0) ? (OLED_WIDTH / 8) : ((OLED_WIDTH / 8) + 1);

    // Canvas canvas;
    // uint32_t buffer_length = (image_height_bytes * image_width_bytes);
    // uint8_t *buffer = (uint8_t*)malloc(buffer_length);
    // canvas.mirror = CANVAS_MIRROR_NONE;
    // canvas.rotate = CANVAS_ROTATE_0;
    // canvas.height = OLED_WIDTH;
    // canvas.width  = OLED_HEIGHT;
    // canvas.image = buffer;
    // canvas_fill(&canvas, 0x0F);
    ssd1306_reset_cursor(&display);
    // ssd1306_display(&display, canvas.image, buffer_length);
    ssd1306_set_addressing(&display, SSD1306_ADDRESSING_VERTICAL);

    // return 0;


    // SSD1306Dev dev = {i2c1, SSD1306_DISPLAY_ADDR};
    // ssd1306_initialize_device(&dev);
    // // Lower contrast to save power
    // ssd1306_set_contrast(&dev, 0x01);
    // // Configure vertical addressing so that canvas scheme makes sense with how
    // // it's drawn to the screen
    // ssd1306_set_addressing(&dev, SSD1306_ADDRESSING_VERTICAL);


    uint32_t gs_buffer_length = (image_height_bytes * image_width_bytes);
    uint8_t *gs_buffer[3] = {NULL, NULL, NULL};
    Canvas framebuffer[3];

    for(uint32_t i = 0; i < 3; i++) {
        gs_buffer[i] = (uint8_t*)malloc(gs_buffer_length);
        framebuffer[i].mirror = CANVAS_MIRROR_NONE;
        framebuffer[i].rotate = CANVAS_ROTATE_0;
        framebuffer[i].height = OLED_WIDTH;
        framebuffer[i].width  = OLED_HEIGHT;
        framebuffer[i].image = gs_buffer[i];
        canvas_fill(&framebuffer[i], 0x00);
        // canvas_draw_grayscale_bmp_sprite(&framebuffer[i], &(ss_grayscale.bitmap), &sprite, i, offset_x, offset_y);
    }

    // Reset the cursor and clear the screen so we start with a blank slate
    ssd1306_reset_cursor(&display);
    // ssd1306_fill_screen(&dev, 0x00);

    // Print version info to screen, 
    LOG_INFO("   OLED Version: %s\n", OLED_VERSION);
    LOG_INFO(" Common Version: %s\n", COMMON_VERSION);

    // Calculate the ecenter of the screen, to draw the image
    uint32_t offset_y = ((OLED_WIDTH - (SPRITE_WIDTH * sprite.magnify)) / 2);
    uint32_t offset_x = ((OLED_HEIGHT - (SPRITE_WIDTH * sprite.magnify)) / 2);
    
    uint32_t dexNumber = 1;
    uint32_t framestart = 0;
    uint32_t frameend = 0;
    uint32_t frames = 0;
    trigger_update = 1;
    do {
        if(trigger_update) {
            // Select the pokemon sprite
            index_to_sprite(dex_number, &ss, &sprite);
            // index_to_sprite(1, &ss_font, &font_sprite);

            for(uint32_t layer = 0; layer < 3; layer++) {
                canvas_draw_grayscale_bmp_sprite(&framebuffer[layer], &(ss_grayscale.bitmap),
                                                &sprite, layer, offset_x, offset_y);
            }
            trigger_update = 0;
        }

        // frames = 0;
        // framestart = to_ms_since_boot(get_absolute_time());
        // frameend = framestart;
        // while((frameend - framestart) < 1000) {
        for(uint32_t i = 0; i < 3; i++) {
            // We shouldn't have to reset the cursor if the entire screen is
            // being written, the display will automaatically wraparound
            ssd1306_display(&display, gs_buffer[i], gs_buffer_length);
        }
        //     frames += 1;
        //     frameend = to_ms_since_boot(get_absolute_time());
        // }
    } while(true);

    // Cleanup
    // free(buffer);

    for(uint i = 0; i < 3; i++) {
        free(gs_buffer[i]);
        gs_buffer[i] = NULL;
    }

    return success;
}