#include "common/draw/bmpspritesheet.h"
#include "common/draw/canvas.h"
#include "common/drivers/ssd1306.h"
#include "common/logger.h"

#include "project/application.h"
#include "project/resources.h"

#define PIN_I2C1_SDA    26
#define PIN_I2C1_SCL    27

#define I2C_BUS_SPEED_KHZ(x) x * 1000

#define SSD1306_DISPLAY_ADDR    0x3D

#define SPRITE_WIDTH    56
#define SPRITE_HEIGHT   56

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

int32_t application_run()
{
    int32_t success = 0;

    stdio_init_all();

    sleep_ms(1000);

    LOG_INFO("Initializing I2C...\n");
    i2c_init(i2c1, I2C_BUS_SPEED_KHZ(2000));
    gpio_set_function(PIN_I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C1_SDA);
    gpio_pull_up(PIN_I2C1_SCL);

    BmpSpriteSheet ss;
    bmpss_initialize(&ss, red_blue_bmp, red_blue_bmp_size);

    BmpSpriteSheet ss_grayscale;
    bmpss_initialize(&ss_grayscale, red_blue_grayscale_bmp, red_blue_grayscale_bmp_size);

    BmpSpriteSheet ss_font;
    bmpss_initialize(&ss_font, red_blue_font_bmp, red_blue_font_bmp_size);

    Canvas frambuffer[3];

    BmpSprite sprite;
    sprite.height = SPRITE_HEIGHT;
    sprite.width = SPRITE_WIDTH;
    sprite.x = 0;
    sprite.y = 0;
    sprite.invert = 0;

    BmpSprite font_sprite;
    font_sprite.height = 8;
    font_sprite.width = 8;
    font_sprite.x = 0;
    font_sprite.y = 0;
    font_sprite.invert = 0;
    font_sprite.rotate = CANVAS_ROTATE_270;
    font_sprite.magnify = 1;

    uint32_t poke_sprite_magnify = 1;
    uint32_t offset_y = ((OLED_WIDTH - (SPRITE_WIDTH * poke_sprite_magnify)) / 2);
    uint32_t offset_x = ((OLED_HEIGHT - (SPRITE_WIDTH * poke_sprite_magnify)) / 2);
    
    LOG_INFO("Initializing Display...\n");
    SSD1306Dev dev = {i2c1, SSD1306_DISPLAY_ADDR};
    ssd1306_initialize_device(&dev);
    // Lower contrast to save power
    ssd1306_set_contrast(&dev, 0x01);
    // Configure vertical addressing so that canvas scheme makes sense with how
    // it's drawn to the screen
    ssd1306_set_addressing(&dev, SSD1306_ADDRESSING_VERTICAL);

    // Since we are using vertical addressing, swap our height and width parameters
    uint32_t image_width_bytes  = OLED_HEIGHT;
    uint32_t image_height_bytes = (OLED_WIDTH % 8 == 0) ? (OLED_WIDTH / 8) : ((OLED_WIDTH / 8) + 1);

    uint32_t buffer_length = (image_height_bytes * image_width_bytes) + 1;
    uint8_t *buffer = (uint8_t*)malloc(buffer_length);

    Canvas canvas;
    // Screen is drawn from right to left so mirror it horizontally
    canvas.mirror = CANVAS_MIRROR_NONE;
    canvas.rotate = 0;
    canvas.height = OLED_WIDTH;
    canvas.width  = OLED_HEIGHT;
    canvas.image  = &buffer[1];
    // Clear the canvas
    canvas_fill(&canvas, 0xFF);

    ssd1306_write_buffer(&dev, buffer, buffer_length);

    LOG_INFO("   OLED Version: %s\n", OLED_VERSION);
    LOG_INFO(" Common Version: %s\n", COMMON_VERSION);

    uint32_t dexNumber = 1;
    do {
        uint64_t start = to_us_since_boot(get_absolute_time());
        // Reset the cursor to the starting point
        ssd1306_reset_cursor(&dev);
        
        if((dexNumber < 1) || (dexNumber > 151)) {
            // Reset the dex counter if we go out of bounds
            dexNumber = 1;
        } 

        // Select the pokemon sprite
        sprite.height = SPRITE_HEIGHT;
        sprite.width = SPRITE_WIDTH;
        sprite.magnify = poke_sprite_magnify;
        sprite.invert = 0;
        sprite.rotate = CANVAS_ROTATE_270;
        index_to_sprite(dexNumber, &ss, &sprite);
        index_to_sprite(1, &ss_font, &font_sprite);

        // Draw the sprite to the canvas and flip bytes
        canvas_draw_bmp_sprite(&canvas, &(ss.bitmap), &sprite, offset_x, offset_y);
        // canvas_draw_bmp_sprite(&canvas, &(ss_font.bitmap), &font_sprite, offset_x, offset_y);

        // Draw to screen
        ssd1306_write_buffer(&dev, buffer, buffer_length);
        uint64_t end = to_us_since_boot(get_absolute_time());
        LOG_INFO("Render time: %llu\n", end - start);

        dexNumber++;

        sleep_ms(1000);
    } while(true);

    return success;
}