#include "project/application.h"

const uint32_t Application::pin_display_cd      = 0;    // DC
const uint32_t Application::pin_spi0_cs         = 1;    // CS
const uint32_t Application::pin_spi0_sck        = 2;    // CLK
const uint32_t Application::pin_spi0_mosi       = 3;    // DIN
const uint32_t Application::pin_display_busy    = 4;    // BUSY
const uint32_t Application::pin_display_reset   = 5;    // RST
const uint32_t Application::pin_generate_fact   = 7;
const uint32_t Application::pin_neopixel        = 16;

const uint32_t Application::neopixel_num_leds       = 1;
const uint32_t Application::neopixel_max_brightness = 25;

static bool draw_new_fact = false;
static uint32_t debounce_generate_fact = to_ms_since_boot(get_absolute_time());
static const uint32_t debounce_delay_time = 1000;

void generate_fact(uint gpio, uint32_t events)
{
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());
    if(gpio == Application::pin_generate_fact) {
        if(!draw_new_fact && 
            (currentTime - debounce_generate_fact) > debounce_delay_time) {
            draw_new_fact = true;
        } else {
            printf("Core busy\n");
        }
    }
}

Application::Application() :
    mImage(nullptr),
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
    UWORD Imagesize = ((EPD_1IN54_V2_WIDTH % 8 == 0)? (EPD_1IN54_V2_WIDTH / 8 ): (EPD_1IN54_V2_WIDTH / 8 + 1)) * EPD_1IN54_V2_HEIGHT;
    if((mImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        // return -1;
    }
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

    // Initialize IRQ
    gpio_set_irq_enabled_with_callback(pin_generate_fact, GPIO_IRQ_EDGE_RISE, true, &generate_fact);

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
    // LOG_INFO("Black out display...\n");
    // mEPaper.fillScreen(0x00);
    LOG_INFO("White out display...\n");
    mEPaper.fillScreen(0xFF);
    LOG_INFO("Placing display in sleep mode...\n");
    mEPaper.sleep();

    int32_t seed = to_us_since_boot(get_absolute_time());
    LOG_INFO("Initializing rand with %d...\n", seed);
    srand(seed);

    LOG_INFO("Initializing console...\n");
    mCmdHelp.addControlObject(&mHandler);

    mHandler.addCommand(&mCmdHelp);
    console_set_command_handler(&mHandler);

    initializeDrawing();

    // Display GREEN to show end of initialization
    mNeopixel.fill(WS2812::RGB(0, 1, 0));
    mNeopixel.show();
}

void Application::initializeDrawing()
{
    drawFact();
}

void Application::drawFact()
{
    int32_t fact = rand() % 1000;
    char title[18] = {};
    snprintf(title, 18, "Snapple Fact #%03d", fact);

    Paint_NewImage(mImage, EPD_1IN54_V2_WIDTH, EPD_1IN54_V2_HEIGHT, 270, WHITE);
    Paint_Clear(WHITE);
    Paint_DrawString_EN(0, 0, title, &Font16, WHITE, BLACK);
    Paint_DrawLine(0, 16, 200, 16, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawString_EN(0, 18, Snapple::facts[fact], &Font16, WHITE, BLACK);
    mEPaper.reset();
    // mEPaper.wake();
    mEPaper.display(mImage);
    mEPaper.sleep();
}

int32_t Application::run()
{
    int32_t error = 0;
    initialize();
    LOG_INFO("Project Version: %s\n", EPAPER_VERSION);
    LOG_INFO(" Common Version: %s\n", COMMON_VERSION);
    LOG_INFO("       App Size: %d bytes\n", sizeof(Application));
    multicore_launch_core1(console_run);
    
    while(true) {
        sleep_ms(100);
        if(draw_new_fact) {
            drawFact();
            draw_new_fact = false;
        }
    }

    return 0;
}