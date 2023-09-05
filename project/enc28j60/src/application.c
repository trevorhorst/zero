#include <stdlib.h>
#include <errno.h>

#include "core/logger.h"
#include "core/console/console.h"
#include "core/drivers/enc28j60.h"
#include "core/drivers/ws2812.h"

#include "project/application.h"

#define PIN_SPI0_CS         1      // CS
#define PIN_SPI0_SCK        2      // CLK
#define PIN_SPI0_MOSI       3      // SI
#define PIN_SPI0_MISO       4      // SO
#define PIN_ETHCTL_RESET    5      // RST
#define PIN_NEOPIXEL        16     // WS2812 Neopixel

#define UNIT_MHZ(x) x * 1000000

enc28j60_spi_device ethernet_controller;

void initialize_pinmux()
{
    // Initialize SPI pins
    gpio_set_function(PIN_SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI0_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI0_MISO, GPIO_FUNC_SPI);

    // Configure pin 1 for chip select on the SPI
    gpio_init(PIN_SPI0_CS);
    gpio_set_dir(PIN_SPI0_CS, GPIO_OUT);
    gpio_put(PIN_SPI0_CS, 1);

    // Configure pin 5 for chip select on the SPI
    gpio_init(PIN_ETHCTL_RESET);
    gpio_set_dir(PIN_ETHCTL_RESET, GPIO_OUT);
    gpio_put(PIN_ETHCTL_RESET, 1);
}

void initialize_neopixel(ws2812_device *device)
{
    device->pin = 16;
    device->length = 1;
    device->pio = pio0;
    device->sm = 0;
    device->bytes[0] = WS2812_DATA_BYTE_NONE;
    device->bytes[1] = WS2812_DATA_BYTE_GREEN;
    device->bytes[2] = WS2812_DATA_BYTE_RED;
    device->bytes[3] = WS2812_DATA_BYTE_BLUE;

    // Initialie the pixel to a blank state
    ws2812_initialize(device);
    ws2812_fill(device, WS2812_RGB(0, 0, 0));
    ws2812_show(device);
}

void initialize_ethernet_controller(enc28j60_spi_device *device)
{
    LOG_INFO("Initialize Ethernet Controller\n");
    uint8_t value = 0;

    // Initialize device settings
    device->bus = spi0;
    device->cs = PIN_SPI0_CS;
    device->reset = PIN_ETHCTL_RESET;

    spi_init(spi0, UNIT_MHZ(10));
    spi_set_format(
        spi0,
        8,
        SPI_CPOL_1,
        SPI_CPHA_1,
        SPI_MSB_FIRST
    );

    enc28j60_spi_read(device, ECON1, &value, sizeof(value));

    LOG_INFO("READ: 0x%04X\n", value);
}

uint32_t spi_reset(int32_t argc, char **argv)
{
    uint32_t error = 0;
    gpio_set_dir(PIN_ETHCTL_RESET, GPIO_OUT);
    gpio_put(PIN_ETHCTL_RESET, 0);
    sleep_ms(1000);
    gpio_put(PIN_ETHCTL_RESET, 1);
    return error;
}

uint32_t spi_read(int32_t argc, char **argv)
{
    uint32_t error = 0;
    LOG_INFO("Read args: %d\n", argc);
    if(argc == 3) {
        // We are performing a read operation
        long address = strtol(argv[2], NULL, 16);
        if(errno == ERANGE) {
            return 1;
        }

        uint8_t value = 0;
        enc28j60_spi_read(&ethernet_controller, address, &value, sizeof(value));
        LOG_INFO("READ: 0x%04X\n", value);
    }
    return error;
}

uint32_t spi_write(int32_t argc, char **argv)
{
    uint32_t error = 0;
    LOG_INFO("Write args: %d\n", argc);
    if(argc == 5) {
        long address = strtol(argv[2], NULL, 16);
        if(errno == ERANGE) {
            return 1;
        }

        long data = strtol(argv[4], NULL, 16);
        if(errno == ERANGE) {
            return 1;
        }

        enc28j60_spi_write_control(&ethernet_controller, address, data);
    }
    return error;
}

#define CMD_SPI_READ    "read"
#define CMD_SPI_WRITE   "write"
#define CMD_SPI_RESET   "reset"

int32_t application_run()
{
    int32_t success = 0;
    stdio_init_all();

    sleep_ms(1000);

    initialize_pinmux();

    // Initialize neopixel device information
    ws2812_device neopixel;
    initialize_neopixel(&neopixel);

    initialize_ethernet_controller(&ethernet_controller);

    LOG_INFO("ENC28J60 Version: %s\n", ENC28J60_VERSION);
    LOG_INFO("  Common Version: %s\n", CORE_VERSION);

    struct console_command cmd_spi_read = {&spi_read};
    struct console_command cmd_spi_write = {&spi_write};
    struct console_command cmd_spi_reset = {&spi_reset};

    // Initialize and launch the console on second core
    console_initialize();
    console_add_command(CMD_SPI_READ, &cmd_spi_read);
    console_add_command(CMD_SPI_WRITE, &cmd_spi_write);
    console_add_command(CMD_SPI_RESET, &cmd_spi_reset);
    multicore_launch_core1(&console_run);

    // Main thread loop
    bool toggle = true;
    while(true) {
        sleep_ms(1000);
        if(toggle) {
            ws2812_fill(&neopixel, WS2812_RGB(0, 25, 0));
            ws2812_show(&neopixel);
        } else {
            ws2812_fill(&neopixel, WS2812_RGB(0, 0, 0));
            ws2812_show(&neopixel);
        }
        toggle = !toggle;
    }

    // For posterity
    ws2812_uninitialize(&neopixel);

    return success;
}
