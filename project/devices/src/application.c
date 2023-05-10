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

uint16_t rgb_oled_buffer[128 * 128 * 2] = {0x001F};

uint32_t image_width_bytes  = OLED_HEIGHT;
uint32_t image_height_bytes = (OLED_WIDTH % 8 == 0) ? (OLED_WIDTH / 8) : ((OLED_WIDTH / 8) + 1);

adxl345_i2c_device accelerometer;
ssd1306_i2c_device display;
ssd1351_spi_device oled;
at24cxxx_i2c_device eeprom;

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


    spi_init(bus, BUS_SPEED_KHZ(10000));
    spi_set_format(
        bus,
        8,
        SPI_CPOL_1,
        SPI_CPHA_1,
        SPI_MSB_FIRST
    );
}

void initialize_oled(ssd1351_spi_device *device)
{
    LOG_INFO("Initializing OLED...\n");
    device->cs = PIN_SPI0_CSn;
    device->dc = PIN_DISPLAY_DC;
    device->reset = PIN_DISPLAY_RST;
    device->bus = spi0;

    ssd1351_spi_initialize_device(device);

    memset(rgb_oled_buffer, 0x55, 128 * 128 * 2);

    ssd1351_spi_write_data(device, rgb_oled_buffer, 128 * 128 * 2);

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

    render_splash_screen();
    sleep_ms(2000);

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

    console_initialize();
    console_add_command(CMD_I2C, &cmd_i2c);
    console_add_command(CMD_EEPROM, &cmd_eeprom);
    console_add_command(CMD_DISPLAY, &cmd_display);
    console_add_command(CMD_DATA, &cmd_accelerometer);
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
