#include "project/application.h"

const uint32_t Application::pin_display_cd      = 0;    // DC
const uint32_t Application::pin_spi0_cs         = 1;    // CS
const uint32_t Application::pin_spi0_sck        = 2;    // CLK
const uint32_t Application::pin_spi0_mosi       = 3;    // DIN
const uint32_t Application::pin_display_busy    = 4;    // BUSY
const uint32_t Application::pin_display_reset   = 5;    // RST
const uint32_t Application::pin_neopixel        = 16;

const uint32_t Application::neopixel_num_leds       = 1;
const uint32_t Application::neopixel_max_brightness = 25;

Application::Application() :
    mEPaper(
        spi0,
        pin_spi0_cs,
        pin_display_cd,
        pin_display_busy,
        pin_display_reset
    ),
    mNeopixel(
        pin_neopixel,
        neopixel_num_leds,
        pio0,
        0,
        WS2812::DataFormat::FORMAT_GRB
    )
{
}

void Application::initialize()
{
    stdio_init_all();
    
    // Give serial some time to come up
    sleep_ms(1000);

    // Just configure debug to show all messages for now
    log_set_level(LOG_TRACE);

    LOG_INFO("\nSTART\n");

    // Display RED to show start of initialization
    mNeopixel.fill(WS2812::RGB(1, 0, 0));
    mNeopixel.show();

    LOG_INFO("Initializing GPIO...\n");

    // Initialize SPI pins
    gpio_set_function(pin_spi0_sck, GPIO_FUNC_SPI);
    gpio_set_function(pin_spi0_mosi, GPIO_FUNC_SPI);

    // Configure pin 0 for command/data on the display
    gpio_init(pin_display_cd);
    gpio_set_dir(pin_display_cd, GPIO_OUT);
    gpio_put(pin_display_cd, 0);

    // Configure pin 1 for chip select on the SPI
    gpio_init(pin_spi0_cs);
    gpio_set_dir(pin_spi0_cs, GPIO_OUT);
    gpio_put(pin_spi0_cs, 0);

    // Configure pin 4 for the busy line on the display
    gpio_init(pin_display_busy);
    gpio_set_dir(pin_display_busy, GPIO_IN);

    // Configure pin 5 for chip select on the SPI
    gpio_init(pin_display_reset);
    gpio_set_dir(pin_display_reset, GPIO_OUT);
    gpio_put(pin_display_reset, 1);

    LOG_INFO("Initializing SPI...\n");

    // Initialize the SPI port to 
    spi_init(spi0, UNIT_MHZ(10));
    spi_set_format(
        spi0,
        8,
        SPI_CPOL_1,
        SPI_CPHA_1,
        SPI_MSB_FIRST
    );

    LOG_INFO("Initializing display...\n");

    mEPaper.initialize();
    LOG_INFO("Black out display...\n");
    mEPaper.fillScreen(0x00);
    LOG_INFO("White out display...\n");
    mEPaper.fillScreen(0xFF);
    LOG_INFO("Placing display in sleep mode...\n");
    mEPaper.sleep();

    // Display GREEN to show end of initialization
    mNeopixel.fill(WS2812::RGB(0, 1, 0));
    mNeopixel.show();
}

int32_t Application::run()
{
    int32_t error = 0;

    initialize();

    console_run();
    return 0;
}