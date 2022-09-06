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

    LOG_INFO("Initializing Display...\n");
    // SSD1306Dev dev = {i2c1, SSD1306_DISPLAY_ADDR};
    // ssd1306_initialize_device(&dev);

    BmpSpriteSheet ss;
    bmpss_initialize(&ss, red_blue_bmp, red_blue_bmp_size);

    BmpSprite sprite;
    sprite.height = SPRITE_HEIGHT;
    sprite.width = SPRITE_WIDTH;
    sprite.x = 0;
    sprite.y = 0;
    sprite.invert = 0;

    uint32_t poke_sprite_magnify = 2;
    uint32_t offset_x = ((OLED_HEIGHT - (SPRITE_HEIGHT * poke_sprite_magnify)) / 2);
    uint32_t offset_y = ((OLED_WIDTH - (SPRITE_WIDTH * poke_sprite_magnify)) / 2) - 10;
    
    SSD1306 display = SSD1306(i2c1, SSD1306_DISPLAY_ADDR);
    display.initialize();

    SSD1306::DisplayRamWrite buffer_0;
    display.fill_display(buffer_0.ram, 0xFF);

    SSD1306::DisplayRamWrite buffer_1;
    display.fill_display(buffer_1.ram, 0xFC);

    SSD1306::DisplayRamWrite buffer_2;
    display.fill_display(buffer_2.ram, 0xF0);

    SSD1306::DisplayRamWrite buffer_3;
    display.fill_display(buffer_3.ram, 0xC0);

    // uint32_t buffer_length = (OLED_HEIGHT * OLED_WIDTH) + 1;
    // uint8_t *buffer = (uint8_t*)malloc(buffer_length);

    // Canvas canvas;
    // canvas.height = OLED_HEIGHT;
    // canvas.width  = OLED_WIDTH;
    // canvas.image  = &buffer[1];
    // canvas_fill(&canvas, 0xF0);
    // canvas.image = buffer;
    // canvas_print(&canvas);

    // LOG_INFO("Ignore display RAM...\n");
    // ssd1306_ignore_ram(&dev, true);
    // sleep_ms(500);
    // LOG_INFO("Acknowledge display RAM...\n");
    // ssd1306_ignore_ram(&dev, false);
    // sleep_ms(500);

    LOG_INFO("Display buffer\n");
    display.set_addressing_mode(SSD1306::AddressingMode::VERTICAL);
    display.write_buffer(buffer_1);
    // display.write_buffer((uint8_t*)&(buffer_1.ram), sizeof(SSD1306::DisplayRam));
    // ssd1306_write_buffer(&dev, canvas.image, buffer_length);

    uint32_t count = 0;
    uint32_t multiplier = 1;
    while(true) {
        if(count == 25) {
            multiplier = -1;
        } else if(count == 1) {
            multiplier = 1;
        }
        LOG_INFO("Contrast: %d\n", count);
        display.set_contrast(count * 10);
        // ssd1306_set_contrast(&dev, count * 10);
        count = count + multiplier;
        sleep_ms(50);
    }

    return success;
}