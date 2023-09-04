#include <stdlib.h>

#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/spi.h>
#include <hardware/dma.h>

#include "core/console/console.h"
#include "core/drivers/ssd1351.h"
#include "core/logger.h"

#include "project/application.h"

// SPI Pins
#define PIN_DISPLAY_DC  0   // DC
#define PIN_SPI0_CSn    1   // CS
#define PIN_SPI0_SCK    2   // CLK
#define PIN_SPI0_MOSI   3   // DIN
#define PIN_DISPLAY_RST 6   // RST

#define BUS_SPEED_KHZ(x) x * 1000

#define RGB_OLED_WIDTH  128
#define RGB_OLED_HEIGHT 128

#define CMD_VERSION     "version"
#define CMD_FILL        "fill"
#define CMD_DMA         "dma"

ssd1351_spi_device rgb_oled;
uint16_t rgb_oled_display_buffer[RGB_OLED_WIDTH * RGB_OLED_HEIGHT];

static volatile uint32_t dma_tx;
static dma_channel_config dma_config;

int32_t version(int32_t argc, char **argv)
{
    int32_t error = 0;
    CONSOLE_UNUSED_ARGS(argc, argv);

    LOG_INFO("Application Version: %s\n", APPLICATION_VERSION);
    LOG_INFO("     Common Version: %s\n", CORE_VERSION);

    return error;
}

/**
 * @brief Initialize GPIO and their functionality
 */
void initialize_pinmux()
{
    LOG_INFO("Initializing pinmux...\n");

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

    // Configure pin 5 for chip select on the SPI
    gpio_init(PIN_DISPLAY_RST);
    gpio_set_dir(PIN_DISPLAY_RST, GPIO_OUT);
    gpio_put(PIN_DISPLAY_RST, 1);
}

/**
 * @brief Initialize the SPI bus
 * @param bus
 */
void initialize_spi()
{
    LOG_INFO("Initializing SPI... \n");
    spi_inst_t *bus = spi0;
    spi_init(bus, BUS_SPEED_KHZ(50000));
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
    LOG_INFO("Initializing OLED...\n");
    rgb_oled.cs = PIN_SPI0_CSn;
    rgb_oled.dc = PIN_DISPLAY_DC;
    rgb_oled.reset = PIN_DISPLAY_RST;
    rgb_oled.bus = spi0;

    ssd1351_spi_initialize_device(&rgb_oled);

    uint16_t color = SSD1351_RGB_65K(0x00, 0x3F, 0x1F);
    for(int32_t i = 0; i < RGB_OLED_HEIGHT; i++) {
        for(int32_t j = 0; j < RGB_OLED_WIDTH; j++) {
            ssd1351_spi_write_data(&rgb_oled, (uint8_t*)&color, sizeof(color));
        }
    }
}

void initialize_dma()
{
    LOG_INFO("Initialize DMA...\n");
    dma_tx = dma_claim_unused_channel(true);
    dma_config = dma_channel_get_default_config(dma_tx);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_dreq(&dma_config, DREQ_SPI0_TX);
    dma_channel_configure(
                dma_tx,
                &dma_config,
                &spi_get_hw(spi0)->dr,
                rgb_oled_display_buffer,
                sizeof(rgb_oled_display_buffer),
                true);
}

int32_t fill_display_operation(int32_t argc, char **argv)
{
    int32_t error = 0;

    if(argc == 2) {
        uint16_t val = (int16_t)strtol(argv[1], NULL, 16);

        /// @note It is significantly faster to draw to a buffer first and then
        /// render the buffer to display. Rather than write a single pixel at a time.
        uint64_t draw_start_us = to_us_since_boot(get_absolute_time());
        for(int32_t i = 0; i < RGB_OLED_HEIGHT; i++) {
            for(int32_t j = 0; j < RGB_OLED_WIDTH; j++) {
                rgb_oled_display_buffer[(RGB_OLED_HEIGHT * i) + j] = val;
            }
        }
        uint64_t draw_stop_us = to_us_since_boot(get_absolute_time());

        uint64_t start_us = to_us_since_boot(get_absolute_time());
        ssd1351_spi_write_data(&rgb_oled, (uint8_t*)rgb_oled_display_buffer, sizeof(rgb_oled_display_buffer));
        uint64_t stop_us = to_us_since_boot(get_absolute_time());

        LOG_INFO("  Time to draw: %llu\n", (draw_stop_us - draw_start_us));
        LOG_INFO("Time to render: %llu\n", (stop_us - start_us));

    } else {
        LOG_INFO("Parameter mismatch\n");
        error = -1;
    }

    return error;
}

int32_t fill_display_dma(int32_t argc, char **argv)
{
    int32_t error = 0;
    if(argc == 2) {
        uint16_t val = (int16_t)strtol(argv[1], NULL, 16);

        /// @note It is significantly faster to draw to a buffer first and then
        /// render the buffer to display. Rather than write a single pixel at a time.
        uint64_t draw_start_us = to_us_since_boot(get_absolute_time());
        for(int32_t i = 0; i < RGB_OLED_HEIGHT; i++) {
            for(int32_t j = 0; j < RGB_OLED_WIDTH; j++) {
                rgb_oled_display_buffer[(RGB_OLED_HEIGHT * i) + j] = val;
            }
        }
        uint64_t draw_stop_us = to_us_since_boot(get_absolute_time());

        // It is possible to call this whilst the DMA is still dumping the previous
        // frame. This blocks until we are ready to start the next frame.
        dma_channel_wait_for_finish_blocking(dma_tx);

        uint64_t start_us = to_us_since_boot(get_absolute_time());
        ssd1351_spi_set_write_ram(&rgb_oled);
        gpio_put(rgb_oled.dc, 1);
        gpio_put(rgb_oled.cs, 0);
        dma_channel_transfer_from_buffer_now(dma_tx, rgb_oled_display_buffer, sizeof(rgb_oled_display_buffer));
        gpio_put(rgb_oled.cs, 1);
        // ssd1351_spi_write_data(&rgb_oled, (uint8_t*)rgb_oled_display_buffer, sizeof(rgb_oled_display_buffer));
        uint64_t stop_us = to_us_since_boot(get_absolute_time());

        LOG_INFO("  Time to draw: %llu\n", (draw_stop_us - draw_start_us));
        LOG_INFO("Time to render: %llu\n", (stop_us - start_us));

    } else {
        LOG_INFO("Parameter mismatch\n");
        error = -1;
    }

    return error;
}

int32_t application_run()
{
    int32_t error = 0;

    stdio_init_all();

    // It seems to help if we wait a little bit after initializing
    sleep_ms(1000);

    // Initialize
    initialize_pinmux();
    initialize_spi();
    initialize_display();
    initialize_dma();

    struct console_command cmd_version = {&version};
    struct console_command cmd_fill    = {&fill_display_operation};
    struct console_command cmd_dma     = {&fill_display_dma};

    console_initialize();
    console_add_command(CMD_VERSION, &cmd_version);
    console_add_command(CMD_FILL, &cmd_fill);
    console_add_command(CMD_DMA, &cmd_dma);
    multicore_launch_core1(&console_run);

    version(0, NULL);

    while(true) {
        sleep_ms(1000);
    }

    return error;
}
