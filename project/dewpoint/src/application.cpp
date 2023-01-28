#include <math.h>

#include "hardware/i2c.h"
#include "hardware/spi.h"

#include "common/logger.h"
#include "common/drivers/ssd1306.h"

#include "project/application.h"
#include "project/resources.h"

// I2C Pins
#define PIN_I2C1_SDA    4
#define PIN_I2C1_SCL    5

// SPI Pins
#define PIN_SPI_MOSI    3
#define PIN_SPI_SCK     2
#define PIN_DISPLAY_RES 26
#define PIN_DISPLAY_DC  27
#define PIN_DISPLAY_CS  28

#define BUS_SPEED_KHZ(x) x * 1000

const char empty_line[]                 = "                ";
const char temp_line[]                  = "Temp    : %.2f ";
const char relative_humidity_line[]     = "RelH    : %.2f ";
const char dewpoint_line[]              = "Dewpoint: %.2f ";
const char frostpoint_line[]            = "Frost   : %.2f ";

bool i2c_reserved_addr(uint8_t addr)
{   
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}  

void i2c_bus_scan(i2c_inst_t *bus)
{
    uint8_t rxdata = 0;

    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if(i2c_reserved_addr(addr)) {
            ret = PICO_ERROR_GENERIC;
        } else {
            ret = i2c_read_blocking(bus, addr, &rxdata, 1, false);
        }

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}


void initialize_i2c(i2c_inst_t *bus)
{
    LOG_INFO("Initializing I2C...\n");
    i2c_init(bus, BUS_SPEED_KHZ(100));
    gpio_set_function(PIN_I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C1_SDA);
    gpio_pull_up(PIN_I2C1_SCL);

    i2c_bus_scan(bus);
}

void initialize_spi(spi_inst_t *bus)
{
    LOG_INFO("Initialize SPI...\n");
    
    // Configure clk and data pins
    gpio_set_function(PIN_SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI_MOSI, GPIO_FUNC_SPI);

    // Initialize the SPI port to 
    spi_init(bus, BUS_SPEED_KHZ(1000));
    spi_set_format(
        bus,
        8,
        SPI_CPOL_1,
        SPI_CPHA_1,
        SPI_MSB_FIRST
    );
}

void initialize_display()
{
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


}

float temperature_in_c(uint16_t temp)
{
    float temperature = -45.0 + (175. * ((float)temp / 65535.));
    return temperature;
}

float temperature_in_f(uint16_t temp)
{
    float temperature = -49.0 + (315. * ((float)temp / 65535.));
    return temperature;
}

float c_to_f(float temp)
{
    return ((9 * temp) / 5) + 32;
}

float relative_humidity(uint16_t rawrh)
{
    float rh = (100.0 * ((float)rawrh / 65535.));
    return rh;
}

float dewpoint_in_c(float temp, float rh)
{
    float a = 17.27;
    float b = 237.7;

    float alpha = ((a * temp) / (b + temp)) + log(rh / 100.0);
    return (b * alpha) / (a - alpha);
}

void write_symbol_to_display(ssd1306_spi_device *display, const uint8_t val)
{
    const char *symbol = getSymbol(val);
    uint8_t map[8];
    memset(&map, 0, 8);
    memcpy(&map[0], symbol, SYMBOL_LENGTH);
    ssd1306_display(display, map, 8);
}

void write_string_to_display(ssd1306_spi_device *display, const char *str )
{
    for(unsigned int i = 0; i < strlen( str ); i++ ) {
        write_symbol_to_display(display, (unsigned char)str[ i ]);
    }
}

int32_t application_run()
{
    int32_t success = 0;
    stdio_init_all();

    sleep_ms(1000);

    initialize_i2c(i2c0);
    initialize_spi(spi0);
    initialize_display();

    ssd1306_spi_device display;
    display.bus   = spi0;
    display.cs    = PIN_DISPLAY_CS;
    display.dc    = PIN_DISPLAY_DC;
    display.reset = PIN_DISPLAY_RES;

    LOG_INFO("Initializing Display...\n");
    ssd1306_initialize_device(&display);
    // ssd1306_set_ignore_ram(&display, true);
    // sleep_ms(500);
    // ssd1306_set_ignore_ram(&display, false);

    // Print version info to screen, 
    LOG_INFO("Dewpoint Version: %s\n", DEWPOINT_VERSION);
    LOG_INFO("  Common Version: %s\n", COMMON_VERSION);

    for(uint8_t i = 0; i < 8; i++) {
        write_string_to_display(&display, empty_line);
    }

    char temperature_text[17];
    char relative_humidity_text[17];
    char dewpoint_text[17];

    while(true) {
        ssd1306_reset_cursor(&display);

        memset(temperature_text, 0x20, 17);

        uint8_t command[] = {0x2C, 0x06};
        i2c_write_blocking(i2c0, 0x44, command, 2, false);
        sleep_ms(1000);
        uint8_t response[6];
        i2c_read_blocking(i2c0, 0x44, response, 6, false);

        int16_t rawtemp = (response[0] << 8) | response[1];
        int16_t rawrh   = (response[3] << 8) | response[4];

        float temp_c = temperature_in_c(rawtemp);
        float temp_f = temperature_in_f(rawtemp);
        float rh     = relative_humidity(rawrh);
        float dp     = dewpoint_in_c(temp_c, rh);

        sprintf(temperature_text, temp_line, temp_f);
        sprintf(relative_humidity_text, relative_humidity_line, rh);
        sprintf(dewpoint_text, dewpoint_line, c_to_f(dp));

        write_string_to_display(&display, temperature_text);
        write_string_to_display(&display, relative_humidity_text);
        write_string_to_display(&display, dewpoint_text);

        printf("Read Temp %fC (%fF)  (0x%02X 0x%02X 0x%02X)\n", temp_c, temp_f, response[0], response[1], response[2]);
        printf("Read RelH %f  (0x%02X 0x%02X 0x%02X)\n", rh, response[3], response[4], response[5]);
        printf("Dew Point: %fC (%fF)\n", dp, c_to_f(dp));
        sleep_ms(1000);
    }

    return success;
}