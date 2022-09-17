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
    i2c_init(i2c1, I2C_BUS_SPEED_KHZ(1000));
    gpio_set_function(PIN_I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C1_SDA);
    gpio_pull_up(PIN_I2C1_SCL);

    // Initialize our sprite sheets
    BmpSpriteSheet ss;
    bmpss_initialize(&ss, red_blue_bmp, red_blue_bmp_size);

    BmpSpriteSheet ss_grayscale;
    bmpss_initialize(&ss_grayscale, red_blue_grayscale_bmp, red_blue_grayscale_bmp_size);

    BmpSpriteSheet ss_font;
    bmpss_initialize(&ss_font, red_blue_font_bmp, red_blue_font_bmp_size);

    // Initialize our sprites
    BmpSprite sprite;
    sprite.height = SPRITE_HEIGHT;
    sprite.width = SPRITE_WIDTH;
    sprite.invert = 0;
    sprite.magnify = 1;
    sprite.rotate = CANVAS_ROTATE_270;

    BmpSprite font_sprite;
    font_sprite.height = 8;
    font_sprite.width = 8;
    font_sprite.x = 56;
    font_sprite.y = 56;
    font_sprite.invert = 0;
    font_sprite.rotate = CANVAS_ROTATE_270;
    font_sprite.magnify = 1;

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

    uint32_t gs_buffer_length = (image_height_bytes * image_width_bytes) + 1;
    uint8_t *gs_buffer[3] = {NULL, NULL, NULL};
    Canvas framebuffer[3];

    for(uint32_t i = 0; i < 3; i++) {
        gs_buffer[i] = (uint8_t*)malloc(gs_buffer_length);
        framebuffer[i].mirror = CANVAS_MIRROR_NONE;
        framebuffer[i].rotate = CANVAS_ROTATE_0;
        framebuffer[i].height = OLED_WIDTH;
        framebuffer[i].width  = OLED_HEIGHT;
        framebuffer[i].image = &gs_buffer[i][1];
        canvas_fill(&framebuffer[i], 0x00);
        // canvas_draw_grayscale_bmp_sprite(&framebuffer[i], &(ss_grayscale.bitmap), &sprite, i, offset_x, offset_y);
    }

    // Reset the cursor and clear the screen so we start with a blank slate
    ssd1306_reset_cursor(&dev);
    ssd1306_fill_screen(&dev, 0x00);

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
    do {
        if((dexNumber < 1) || (dexNumber > 151)) {
            // Reset the dex counter if we go out of bounds
            dexNumber = 1;
        } 

        // Select the pokemon sprite
        index_to_sprite(dexNumber, &ss, &sprite);
        // index_to_sprite(1, &ss_font, &font_sprite);

        for(uint32_t layer = 0; layer < 3; layer++) {
            canvas_draw_grayscale_bmp_sprite(&framebuffer[layer], &(ss_grayscale.bitmap),
                                             &sprite, layer, offset_x, offset_y);
        }

        ssd1306_reset_cursor(&dev);
        frames = 0;
        framestart = to_ms_since_boot(get_absolute_time());
        frameend = framestart;
        while((frameend - framestart) < 1000) {
            for(uint32_t i = 0; i < 3; i++) {
                // We shouldn't have to reset the cursor if the entire screen is
                // being written, the display will automaatically wraparound
                ssd1306_write_buffer(&dev, gs_buffer[i], gs_buffer_length);
            }
            frames += 1;
            frameend = to_ms_since_boot(get_absolute_time());
        }

        dexNumber++;

        printf("FPS: %d\n", frames);
    } while(true);

    // Cleanup
    // free(buffer);

    for(uint i = 0; i < 3; i++) {
        free(gs_buffer[i]);
        gs_buffer[i] = NULL;
    }

    return success;
}