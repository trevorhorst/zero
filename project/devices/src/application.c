#include <string.h>

#include "core/console/console.h"
#include "core/draw/bmpspritesheet.h"
#include "core/draw/canvas.h"
#include "core/drivers/adxl345.h"
#include "core/drivers/at24c.h"
#include "core/drivers/ssd1306.h"
#include "core/drivers/ssd1351.h"
#include "core/logger.h"
#include "core/tools/i2c.h"

#include "project/application.h"
#include "project/resources.h"

#include "hardware/i2c.h"
#include "hardware/spi.h"

// SPI Pins
#define PIN_DISPLAY_DC  0   // DC
#define PIN_SPI0_CSn    1   // CS
#define PIN_SPI0_SCK    2   // CLK
#define PIN_SPI0_MOSI   3   // DIN
#define PIN_DISPLAY_RST 6   // RST

// I2C Pins
#define PIN_I2C1_SDA    4
#define PIN_I2C1_SCL    5

#define BUS_SPEED_KHZ(x) x * 1000

#define RGB_OLED_WIDTH  128
#define RGB_OLED_HEIGHT 128

#define CMD_I2C     "scan"
#define CMD_EEPROM  "eeprom"
#define CMD_R32     "r32"
#define CMD_W32     "w32"
#define CMD_D32     "d32"
#define CMD_DISPLAY "display"
#define CMD_DATA    "data"
#define CMD_FILL    "fill"
#define CMD_CONTRAST    "contrast"

uint8_t rgb_oled_buffer[128 * 128 * 2] = {0x001F};

uint32_t image_width_bytes  = OLED_HEIGHT;
uint32_t image_height_bytes = (OLED_WIDTH % 8 == 0) ? (OLED_WIDTH / 8) : ((OLED_WIDTH / 8) + 1);

adxl345_i2c_device accelerometer;
ssd1306_i2c_device display;
ssd1351_spi_device oled;
at24cxxx_i2c_device eeprom;

typedef struct rgb_color_t {
    uint16_t blue : 5;
    uint16_t green : 6;
    uint16_t red : 5;
} rgb_color;

typedef union color_t {
    uint16_t color;
    uint8_t byte[2];
    rgb_color offset;
} color;

typedef enum colors_t {
   COLORS_WHITE  = 0,
   COLORS_BLACK  = 1,
   COLORS_RED    = 2,
   COLORS_YELLOW = 3,
   COLORS_GREEN  = 4,
   COLORS_CYAN   = 5,
   COLORS_BLUE   = 6,
   COLORS_PURPLE = 7
} colors;

color color_table[8] = {
    {.color = SSD1351_RGB_65K(0x1F, 0x3F, 0x1F)},   // White
    {.color = SSD1351_RGB_65K(0x00, 0x00, 0x00)},   // Black
    {.color = SSD1351_RGB_65K(0x1F, 0x00, 0x00)},   // Red
    {.color = SSD1351_RGB_65K(0x1F, 0x3F, 0x00)},   // Yellow
    {.color = SSD1351_RGB_65K(0x00, 0x3F, 0x00)},   // Green
    {.color = SSD1351_RGB_65K(0x00, 0x3F, 0x1F)},   // Cyan
    {.color = SSD1351_RGB_65K(0x00, 0x00, 0x1F)},   // Blue
    {.color = SSD1351_RGB_65K(0x1F, 0x00, 0x1F)},   // Purple
};

void render_splash_screen()
{
    // Initialize sprite sheet from our resources
    BmpSpriteSheet ss;
    bmpss_initialize_from_resource(&ss, sarah_bmp, sarah_bmp_size);

    // Initialize a canvas for rendering
    Canvas ss_canvas;
    canvas_initialize(&ss_canvas, ss.bitmap.info_header.height, ss.bitmap.info_header.width);
    canvas_fill(&ss_canvas, 0x00);

    // Initialize a sprite the size of the bitmap (since the whole bitmap will
    // be our sprite)
    bmp_sprite_view sprite = {.x = 0, .y = 0, .width = ss.bitmap.info_header.width,
                             .height = ss.bitmap.info_header.height, .invert = 0,
                             .rotate = CANVAS_ROTATE_0, .magnify = 1};

    // Draw the sprite to canvas
    canvas_draw_bmp_sprite(&ss_canvas, &(ss.bitmap), &sprite, 0, 0);

    // Draw the canvas to the display
    ssd1306_i2c_write_data(&display, ss_canvas.image, ss_canvas.size);
}

void initialize_i2c(i2c_inst_t *bus)
{
    LOG_INFO("Initializing I2C...\n");
    i2c_init(bus, BUS_SPEED_KHZ(400));
    gpio_set_function(PIN_I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C1_SDA);
    gpio_pull_up(PIN_I2C1_SCL);

    // tools_i2c_bus_scan(bus);
}

void initialize_spi(spi_inst_t *bus)
{
    LOG_INFO("Initializing SPI...\n");

    // Initialize GPIO
    gpio_set_function(PIN_SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI0_MOSI, GPIO_FUNC_SPI);

    // Configure pin 0 for command/data on the display
    gpio_init(PIN_DISPLAY_DC);
    gpio_set_dir(PIN_DISPLAY_DC, GPIO_OUT);
    gpio_put(PIN_DISPLAY_DC, 1);

    // Configure pin 1 for chip select on the SPI
    gpio_init(PIN_SPI0_CSn);
    gpio_set_dir(PIN_SPI0_CSn, GPIO_OUT);
    gpio_put(PIN_SPI0_CSn, 0);

    // // Configure pin 4 for the busy line on the display
    // gpio_init(PIN_DISPLAY_BUSY);
    // gpio_set_dir(PIN_DISPLAY_BUSY, GPIO_IN);

    // Configure pin 5 for chip select on the SPI
    gpio_init(PIN_DISPLAY_RST);
    gpio_set_dir(PIN_DISPLAY_RST, GPIO_OUT);
    gpio_put(PIN_DISPLAY_RST, 1);


    spi_init(bus, BUS_SPEED_KHZ(50000));
    spi_set_format(
        bus,
        8,
        SPI_CPOL_1,
        SPI_CPHA_1,
        SPI_MSB_FIRST
    );
}

void display_color_bars(ssd1351_spi_device *device)
{
    color c = {.color = SSD1351_RGB_65K(0x1F,0x3F,0x1F)};
    for(size_t k = 0; k < 8; k++) {
        c.color = color_table[k].color;
        for(size_t j = 0; j < 16; j++) {
            for(size_t i = 0; i < RGB_OLED_WIDTH; i++) {
                ssd1351_spi_write_data(device, c.byte, 2);
            }
        }
    }
}

void display_color_gradient(ssd1351_spi_device *device)
{
    bool ignore_step_1 = false;
    bool ignore_step_2 = false;
    bool ignore_step_3 = false;

    uint16_t red   = SSD1351_RGB_RED_MAX;
    uint16_t blue  = 0;
    uint16_t green = 0;
    color c = {.color = SSD1351_RGB_65K(red, blue, green)};
    for(size_t i = 0; i < RGB_OLED_HEIGHT; i++) {
        for(size_t j = 0; j < RGB_OLED_WIDTH; j++) {
            ssd1351_spi_write_data(device, c.byte, 2);
        }
        if(!ignore_step_1) {
            red = red - 1;
            blue = blue + 1;
            green = 0;
            c.color = SSD1351_RGB_65K(red, green, blue);
            if(red == 0) {
                ignore_step_1 = true;
            }
        } else if(!ignore_step_2) {
            red = 0;
            if(i & 1) {
                blue = blue - 1;
            }
            green = green + 1;
            c.color = SSD1351_RGB_65K(red, green, blue);
            if(blue == 0) {
                ignore_step_2 = true;
            }
        } else if(!ignore_step_3) {
            if(i & 1) {
                red = red + 1;
            }
            blue = 0;
            green = green - 1;
            c.color = SSD1351_RGB_65K(red, green, blue);
            if(green == 0) {
                ignore_step_3 = true;
            }
        } else {
            red = 0;
            blue = 0;
            green = 0;
            c.color = SSD1351_RGB_65K(red, green, blue);
        }
    }

    // for(size_t k = 0; k < 8; k++) {
    //     c.color = color_table[k].color;
    //     if(k == COLORS_RED) {
    //         for(size_t j = 0; j < 16; j++) {
    //             for(size_t i = 0; i < RGB_OLED_WIDTH; i++) {
    //                 ssd1351_spi_write_data(device, c.byte, 2);
    //             }
    //             // c.color = SSD1351_RGB_65K((SSD1351_RGB_RED_MAX - j), 0x00, 0x00);
    //             c.color = SSD1351_RGB_65K(((SSD1351_RGB_RED_MAX - (j * 2))), 0x00, 0x00);
    //             LOG_INFO("Red color: 0x%02X\n", c.color);
    //         }
    //     } else if(k == COLORS_GREEN) {
    //         for(size_t j = 0; j < 16; j++) {
    //             for(size_t i = 0; i < RGB_OLED_WIDTH; i++) {
    //                 ssd1351_spi_write_data(device, c.byte, 2);
    //             }
    //             c.color = SSD1351_RGB_65K(0x00, ((SSD1351_RGB_GRN_MAX - (j * 4))), 0x00);
    //             LOG_INFO("Blue color: 0x%02X\n", c.color);
    //         }

    //     } else if(k == COLORS_YELLOW) {
    //         for(size_t j = 0; j < 16; j++) {
    //             for(size_t i = 0; i < RGB_OLED_WIDTH; i++) {
    //                 ssd1351_spi_write_data(device, c.byte, 2);
    //             }
    //             c.color = SSD1351_RGB_65K(((j * 2) + 1), 0x00, ((SSD1351_RGB_BLU_MAX - (j * 2))));
    //             LOG_INFO("Blue color: 0x%02X\n", c.color);
    //         }
    //     } else if(k == COLORS_PURPLE) {
    //         for(size_t j = 0; j < 16; j++) {
    //             for(size_t i = 0; i < RGB_OLED_WIDTH; i++) {
    //                 ssd1351_spi_write_data(device, c.byte, 2);
    //             }
    //             c.color = SSD1351_RGB_65K(((j * 2) + 1), 0x00, ((SSD1351_RGB_BLU_MAX - (j * 2))));
    //             LOG_INFO("Blue color: 0x%02X\n", c.color);
    //         }


    //     } else if(k == COLORS_BLUE) {
    //         for(size_t j = 0; j < 16; j++) {
    //             for(size_t i = 0; i < RGB_OLED_WIDTH; i++) {
    //                 ssd1351_spi_write_data(device, c.byte, 2);
    //             }
    //             c.color = SSD1351_RGB_65K(0x00, 0x00, ((SSD1351_RGB_BLU_MAX - (j * 2))));
    //             LOG_INFO("Blue color: 0x%02X\n", c.color);
    //         }

    //     } else {
    //         for(size_t j = 0; j < 16; j++) {
    //             for(size_t i = 0; i < RGB_OLED_WIDTH; i++) {
    //                 ssd1351_spi_write_data(device, c.byte, 2);
    //             }
    //         }
    //     }
    // }
}

void initialize_oled(ssd1351_spi_device *device)
{
    LOG_INFO("Initializing OLED...\n");
    device->cs = PIN_SPI0_CSn;
    device->dc = PIN_DISPLAY_DC;
    device->reset = PIN_DISPLAY_RST;
    device->bus = spi0;

    ssd1351_spi_initialize_device(device);

    color c = {.color = SSD1351_RGB_65K(0x1F,0x3F,0x1F)};
    LOG_INFO("color: 0x%02X\n", c.color);

    // Clear the display!
    memset(rgb_oled_buffer, 0x00, 128 * 128 * 2);
    ssd1351_spi_write_data(device, rgb_oled_buffer, 128 * 128 * 2);

    /// display_color_bars(device);
    display_color_gradient(device);

    // memset(rgb_oled_buffer, 0xFF, 128 * 128 * 2);
    // ssd1351_spi_write_data(device, rgb_oled_buffer, 128 * 2);

    // memset(rgb_oled_buffer, 0x03, 128 * 128 * 2);
    // ssd1351_spi_write_data(device, rgb_oled_buffer, 128 * 2);
    // ssd1351_spi_reset_cursor(device);

    // for(size_t k = 0; k < 8; k++) {
    //     c.color = color_table[k].color;
    //     for(size_t j = 0; j < 16; j++) {
    //         for(size_t i = 0; i < RGB_OLED_WIDTH; i++) {
    //             ssd1351_spi_write_data(device, c.byte, 2);
    //         }
    //     }
    // }
}

void initialize_accelerometer(adxl345_i2c_device *device)
{
    LOG_INFO("Initializing Accelerometer...\n");
    device->address = ADXL345_I2C_ADDRESS(0);
    device->bus     = i2c0;

    uint8_t devid = 0;
    adxl345_i2c_read(device, ADXL345_REG_DEVID, &devid, sizeof(devid));
    LOG_INFO("  Accelerometer (0x%02X) @ 0x%02X\n", devid, device->address);

    adxl345_power_ctl ctl = {.value = 0};
    ctl.f.measure = 1;
    adxl345_i2c_set_power_ctl(device, &ctl);

    adxl345_data_format fmt = {.value = 0};
    fmt.f.range = ADXL345_RANGE_16G;
    adxl345_i2c_set_data_format(device, &fmt);
}

void initialize_display(ssd1306_i2c_device *device)
{
    LOG_INFO("Initializing Display...\n");
    device->address = 0x3C;
    device->bus     = i2c0;

    ssd1306_i2c_initialize_device(&display);
    ssd1306_i2c_reset_cursor(&display);
    ssd1306_i2c_set_addressing(&display, SSD1306_ADDRESSING_VERTICAL);

    // render_splash_screen();
    // sleep_ms(2000);

    ssd1306_i2c_clear_display(&display);
}

void initialize_eeprom(at24cxxx_i2c_device *device)
{
    LOG_INFO("Initializing EEPROM...\n");
    device->address = AT24CXXX_I2C_ADDRESS(0, 1);
    device->bus     = i2c0;
    LOG_INFO("  EEPROM @ 0x%02X\n", device->address);
}

uint32_t i2c_scan(int32_t argc, char **argv)
{
    uint32_t error = 0;
    tools_i2c_bus_scan(i2c0);
    return error;
}

uint32_t eeprom_operation(int32_t argc, char **argv)
{
    uint32_t error = 0;
    LOG_INFO("EEPROM Operation Argc %d\n", argc);
    if(argc == 3) {
        if(strncmp(CMD_R32, argv[1], 128) == 0) {
            // We are performing a read operation
            long address = strtol(argv[2], NULL, 16);
            if(errno == ERANGE) {
                return 1;
            }

            uint8_t buffer = 0x00;
            at24cxxx_random_read(&eeprom, address, &buffer, sizeof(buffer));
            LOG_INFO("0x%02X --> 0x%02X\n", address, buffer);
        }
    } else if(argc == 5) {
        if(strncmp(CMD_W32, argv[1], 128) == 0) {
            // We are performing a read operation
            long address = strtol(argv[2], NULL, 16);
            if(errno == ERANGE) {
                return 1;
            }

            long data = strtol(argv[4], NULL, 16);
            if(errno == ERANGE) {
                return 1;
            }

            uint8_t buffer[3] = {0, (uint8_t)address, (uint8_t)data};
            at24cxxx_byte_write(&eeprom, buffer, sizeof(buffer));
            LOG_INFO("0x%02X <-- 0x%02X\n", address, data);
        }

        if(strncmp(CMD_D32, argv[1], 128) == 0) {
            // We are performing a read operation
            long address = strtol(argv[2], NULL, 16);
            if(errno == ERANGE) {
                return 1;
            }

            long length = strtol(argv[4], NULL, 16);
            if(errno == ERANGE) {
                return 1;
            }

            uint8_t *buffer = (uint8_t*)malloc(sizeof(uint8_t) * length);
            at24cxxx_random_read(&eeprom, address, buffer, length);

            for(int32_t i = 0; i < length; i++) {
                LOG_INFO("0x%02X --> 0x%02X\n", address + i, buffer[i]);
            }

            free(buffer);
        }

    }

    return error;
}

uint32_t display_operation(int32_t argc, char **argv)
{
    uint8_t buffer[256] = {0, 0, 0, 0, 0};
    ssd1306_i2c_read(&display, buffer, sizeof(buffer));
    for(int32_t i = 0; i < sizeof(buffer); i++) {
        printf("0x%02X\n", buffer[i]);
    }

    return 0;
}

uint32_t accelerometer_operation(int32_t argc, char **argv)
{
    uint32_t success = 0;
    // uint8_t devid = 0;
    // adxl345_i2c_read(&accelerometer, ADXL345_REG_DEVID, &devid, sizeof(devid));
    // LOG_INFO("DEVID: 0x%02X\n", devid);
    // adxl345_i2c_get_thresh_tap(&accelerometer);
    adxl345_data data;
    adxl345_i2c_get_data(&accelerometer, &data);
    printf("X: %d   Y: %d   Z: %d\n", data.f.datax, data.f.datay, data.f.dataz);

    // LOG_INFO("DEVID: 0x%02X\n", devid);
    return success;
}

uint32_t fill_operation(int32_t argc, char **argv)
{
    int16_t val = (int16_t)strtol(argv[2], NULL, 16);
    if(argc == 3) {
        LOG_INFO("arg %d\n", val);
    } else {
        return 1;
    }

    color c = {.color = val};
    LOG_INFO("Sizeof color %d\n", sizeof(color));
    LOG_INFO("color: 0x%02X\n", c.color);
    LOG_INFO("  red: 0x%02X\n", c.offset.red);
    LOG_INFO("  grn: 0x%02X\n", c.offset.green);
    LOG_INFO("  blu: 0x%02X\n", c.offset.blue);

    // // Clear the display!
    // memset(rgb_oled_buffer, 0x00, 128 * 128 * 2);
    // ssd1351_spi_write_data(&oled, rgb_oled_buffer, 128 * 128 * 2);

    // memset(rgb_oled_buffer, 0xFF, 128 * 128 * 2);
    // ssd1351_spi_write_data(device, rgb_oled_buffer, 128 * 2);

    // memset(rgb_oled_buffer, 0x03, 128 * 128 * 2);
    // ssd1351_spi_write_data(device, rgb_oled_buffer, 128 * 2);

    for(size_t j = 0; j < RGB_OLED_HEIGHT; j++) {
        for(size_t i = 0; i < RGB_OLED_WIDTH; i++) {
            ssd1351_spi_write_data(&oled, (uint8_t*)&c.color, 2);
        }
    }

    return 0;
}

uint32_t contrast_operation(int32_t argc, char **argv)
{
    int16_t val = (int16_t)strtol(argv[2], NULL, 10);
    if(argc == 3) {
        LOG_INFO("arg %d\n", val);
    } else {
        return 1;
    }

    ssd1351_spi_set_contrast_master(&oled, val);

    return 0;
}

int32_t application_run()
{
    int32_t success = 0;

    stdio_init_all();

    sleep_ms(1000);

    Canvas canvas;
    uint32_t canvas_buffer_length = (image_height_bytes * image_width_bytes + 1);
    uint8_t *canvas_buffer = (uint8_t *)malloc(canvas_buffer_length);
    canvas.mirror = CANVAS_MIRROR_NONE;
    canvas.rotate = CANVAS_ROTATE_0;
    canvas.height = OLED_WIDTH;
    canvas.width  = OLED_HEIGHT;
    canvas.image = &canvas_buffer[1];

    canvas_buffer[0] = OLED_I2C_CONTROL_BYTE(0, 1);
    canvas_fill(&canvas, 0x00);


    initialize_spi(spi0);
    initialize_i2c(i2c0);
    initialize_accelerometer(&accelerometer);
    initialize_display(&display);
    initialize_oled(&oled);
    // initialize_eeprom(&eeprom);


    uint32_t center_x = OLED_HEIGHT / 2;
    uint32_t center_y = OLED_WIDTH / 2;

    uint32_t x = center_x - 2;
    uint32_t y = center_y - 2;

    canvas_draw_line(&canvas, 0, 0, (OLED_HEIGHT - 1), (OLED_WIDTH - 1), CC_WHITE);
    canvas_draw_line(&canvas, (OLED_HEIGHT - 1), 0, 0, (OLED_WIDTH - 1), CC_WHITE);
    canvas_draw_rectangle(&canvas, 0, 0, (OLED_HEIGHT - 1), (OLED_WIDTH - 1), CC_WHITE);
    canvas_draw_circle(&canvas, center_x, center_y, 25, CC_WHITE);
    canvas_draw_circle(&canvas, center_x, center_y, 31, CC_WHITE);
    canvas_draw_point(&canvas, x, y, CC_WHITE, PIXEL_4X4);
    ssd1306_i2c_write(&display, canvas_buffer, canvas_buffer_length);

    struct console_command cmd_i2c = {&i2c_scan};
    struct console_command cmd_eeprom = {&eeprom_operation};
    struct console_command cmd_display = {&display_operation};
    struct console_command cmd_accelerometer = {&accelerometer_operation};
    struct console_command cmd_fill = {&fill_operation};
    struct console_command cmd_contrast = {&contrast_operation};

    console_initialize();
    console_add_command(CMD_I2C, &cmd_i2c);
    console_add_command(CMD_EEPROM, &cmd_eeprom);
    console_add_command(CMD_DISPLAY, &cmd_display);
    console_add_command(CMD_DATA, &cmd_accelerometer);
    console_add_command(CMD_FILL, &cmd_fill);
    console_add_command(CMD_CONTRAST, &cmd_contrast);
    multicore_launch_core1(&console_run);

    LOG_INFO("Devices Version: %s\n", DEVICES_VERSION);
    LOG_INFO(" Common Version: %s\n", CORE_VERSION);

    int32_t x_range  = 28;
    int32_t y_range  = 60;

    while(true) {
        // Clear previous point
        // canvas_draw_point(&canvas, x, y, CC_BLACK, PIXEL_4X4);

        // sleep_ms(10);
        adxl345_data data;
        // adxl345_i2c_get_data(&accelerometer, &data);
        // printf("X: %d   Y: %d   Z: %d\n", data.f.datax, data.f.datay, data.f.dataz);

        x = center_x - data.f.datay;
        if(data.f.datay >= x_range) {
            x = center_x - x_range;
        } else if(data.f.datay <= (0 - x_range)) {
            x = center_x + x_range;
        }

        y = center_y + (data.f.datax * 2);
        if(data.f.datay >= y_range) {
            y = center_y + y_range;
        } else if(data.f.datay <= (0 - y_range)) {
            y = center_y - y_range;
        }

        canvas_draw_point(&canvas, x, y, CC_WHITE, PIXEL_4X4);
        // ssd1306_i2c_write(&display, canvas_buffer, canvas_buffer_length);
    }

    free(canvas.image);

    return success;
}
