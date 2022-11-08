#include "core/drivers/ssd1306.h"
#include "core/console/console.h"
#include "core/version.h"
#include "core/logger.h"

#include "project/application.h"
#include "project/game.h"

#define SPI_BUS_SPEED_KHZ(x) x * 1000

// // Debounce control
#define DEBOUNCE_DELAY_TIME _u(250)
#define PIN_GAME_RESET_BTN  _u(6)
#define PIN_GAME_SPEED_BTN  _u(7)
#define PIN_SPI_MOSI    3
#define PIN_SPI_SCK     2
#define PIN_DISPLAY_RES 26
#define PIN_DISPLAY_DC  27
#define PIN_DISPLAY_CS  28


#define CMD_TEST    "test"
#define CMD_PRINT    "print"
#define CMD_CHECK    "check"
#define CMD_STEP     "step"
#define CMD_BUFFER   "buffer"
#define CMD_RUN     "run"
#define CMD_STOP    "stop"

static conways_game game;
static ssd1306_spi_device display;

static bool running = false;

uint32_t test_callback()
{
    printf("Hello Test\n");
}

uint32_t run_game()
{
    running = true;
}

uint32_t stop_game()
{
    running = false;
}

uint32_t print_board()
{
    print_ram_board(&game);
}

uint32_t print_buffer()
{
    print_ram_buffer(&game);
}

uint32_t check_board()
{
    check_ram_board(&game, true);
}

uint32_t step_board()
{
    print_ram_board(&game);
    check_ram_board(&game, true);

    uint8_t *temp = game.board;
    game.board = game.buffer;
    game.buffer = temp;
    ssd1306_display(&display, game.board, (game.height * game.width));
    print_ram_board(&game);
}

void application_initialize_spi()
{
	LOG_INFO("Initialize SPI...\n");
	
	// Configure clk and data pins
	gpio_set_function(PIN_SPI_SCK, GPIO_FUNC_SPI);
	gpio_set_function(PIN_SPI_MOSI, GPIO_FUNC_SPI);

	// Initialize the SPI port to 
	spi_init(spi0, SPI_BUS_SPEED_KHZ(370*4));
	spi_set_format(
		spi0,
		8,
		SPI_CPOL_1,
		SPI_CPHA_1,
		SPI_MSB_FIRST
	);
}

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    // gpio_event_string(event_str, events);
    static uint32_t debounce_reset_button = 0;// to_ms_since_boot(get_absolute_time());
    static uint32_t debounce_speed_button = 0;// to_ms_since_boot(get_absolute_time());

    // LOG_TRACE("GPIO Reset Event: %u\n", gpio);
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());

    if(gpio == PIN_GAME_RESET_BTN) {
        if((currentTime - debounce_reset_button) > DEBOUNCE_DELAY_TIME) {
            debounce_reset_button = currentTime;
            printf("RESET BUTTON PRESSED\n");
            // conwaysSetReset();
        }
    } else if(gpio == PIN_GAME_SPEED_BTN) {
        if((currentTime - debounce_speed_button) > DEBOUNCE_DELAY_TIME) {
            debounce_speed_button = currentTime;
            printf("SPEED BUTTON PRESSED\n");
            // conwaysStepSpeed();
        }
    }
}

int32_t application_run()
{
    // Initialize the serial interface.. amongst other things
    stdio_init_all();

    // Give the console some time to get setup...
    sleep_ms(1000);
    
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


    application_initialize_spi();
    display.bus   = spi0;
    display.cs    = PIN_DISPLAY_CS;
    display.dc    = PIN_DISPLAY_DC;
    display.reset = PIN_DISPLAY_RES;
    
    LOG_INFO("Initializing Seed...\n");
    uint64_t seed = to_us_since_boot(get_absolute_time());
    LOG_INFO("  Seed is %lu\n", seed);
    // Initialize the random number generator
    srand(seed);

    LOG_INFO("Initializing Display...\n");
    ssd1306_initialize_device(&display);
    ssd1306_set_addressing(&display, SSD1306_ADDRESSING_VERTICAL);

    LOG_INFO("Initializing Game...\n");

    // Since we are using vertical addressing, swap our height and width parameters
    // uint32_t image_width_bytes  = OLED_HEIGHT;
    // uint32_t image_height_bytes = (OLED_WIDTH % 8 == 0) ? (OLED_WIDTH / 8) : ((OLED_WIDTH / 8) + 1);
    uint32_t image_width_bytes  = 2; // (OLED_HEIGHT % 8 == 0) ? (OLED_HEIGHT / 8) : ((OLED_HEIGHT / 8) + 1);
    uint32_t image_height_bytes = 16; // OLED_WIDTH;

    conways_initialize(&game, image_width_bytes, image_height_bytes);
    ssd1306_display(&display, game.board, (game.height * game.width));
    print_ram_board(&game);
    // check_ram_board(&game);

    struct console_command cmd_test = {&test_callback};
    struct console_command cmd_print = {&print_board};
    struct console_command cmd_buffer = {&print_buffer};
    struct console_command cmd_check = {&check_board};
    struct console_command cmd_step = {&step_board};
    struct console_command cmd_run = {&run_game};
    struct console_command cmd_stop = {&stop_game};
    console_initialize();
    console_add_command(CMD_TEST, &cmd_test);
    console_add_command(CMD_PRINT, &cmd_print);
    console_add_command(CMD_BUFFER, &cmd_buffer);
    console_add_command(CMD_CHECK, &cmd_check);
    console_add_command(CMD_STEP, &cmd_step);
    console_add_command(CMD_STOP, &cmd_stop);
    console_add_command(CMD_RUN, &cmd_run);

    // Initialize interrupts
    gpio_set_irq_enabled_with_callback(PIN_GAME_RESET_BTN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(PIN_GAME_SPEED_BTN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    LOG_INFO("Conways Version: %s\n", CONWAYS_VERSION);
    LOG_INFO(" Common Version: %s\n", CORE_VERSION);

    multicore_launch_core1(console_run);

    while(true) {
        // step_board();
        if(running) {
            check_ram_board(&game, false);

            uint8_t *temp = game.board;
            game.board = game.buffer;
            game.buffer = temp;
            ssd1306_display(&display, game.board, (game.height * game.width));
        }
        // conways_run();
        // ssd1306_display(&display, game.board, (game.height * game.width));
        sleep_ms(125);
    }

    return 0;
}

// bool i2c_reserved_addr(uint8_t addr)
// {
//     return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
// }
//
// void i2c_bus_scan(i2c_inst_t *bus)
// {
//     uint8_t rxdata = 0;
//
//     printf("\nI2C Bus Scan\n");
//     printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
//
//     for (int addr = 0; addr < (1 << 7); ++addr) {
//         if (addr % 16 == 0) {
//             printf("%02x ", addr);
//         }
//
//         // Perform a 1-byte dummy read from the probe address. If a slave
//         // acknowledges this address, the function returns the number of bytes
//         // transferred. If the address byte is ignored, the function returns
//         // -1.
//
//         // Skip over any reserved addresses.
//         int ret;
//         uint8_t rxdata;
//         if(i2c_reserved_addr(addr)) {
//             ret = PICO_ERROR_GENERIC;
//         } else {
//             ret = i2c_read_blocking(bus, addr, &rxdata, 1, false);
//         }
//
//         printf(ret < 0 ? "." : "@");
//         printf(addr % 16 == 15 ? "\n" : "  ");
//     }
//     printf("Done.\n");
// }
//
// const uint32_t Application::pin_game_reset  = 2;
// const uint32_t Application::pin_game_speed  = 3;
// const uint32_t Application::pin_i2c1_sda     = 26;
// const uint32_t Application::pin_i2c1_scl     = 27;
//
// const uint8_t Application::ssd1306_display_addr = 0x3D;
//
// Application::Application()
//     : mDisplay(i2c1, ssd1306_display_addr)
// {
//
// }

void application_initialize()
{
    // gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // initializeI2C();
    // initializeDisplay();
    // initializeConsole();
}

// void Application::initializeI2C()
// {
//     LOG_INFO("Initializing I2C...\n");
//     i2c_init(i2c1, I2C_BUS_SPEED_KHZ(1000));
//     gpio_set_function(pin_i2c1_sda, GPIO_FUNC_I2C);
//     gpio_set_function(pin_i2c1_scl, GPIO_FUNC_I2C);
//     gpio_pull_up(pin_i2c1_sda);
//     gpio_pull_up(pin_i2c1_scl);
//
//     // i2c_bus_scan(i2c1);
// }
//
// void Application::initializeDisplay()
// {
//     LOG_INFO("Initializing display...\n");
//     LOG_INFO("Display @ 0x%02X\n", ssd1306_display_addr);
//     mDisplay.initialize();
//
//     // Flash display so we know it's alive
//     mDisplay.fill_screen(0xFF);
//     sleep_ms(500);
//     mDisplay.fill_screen(0x00);
// }
//
// void Application::initializeConsole()
// {
//     mCmdHelp.addControlObject(&mHandler);
//     // mCmdPixel.addControlObject(&mNeopixel);
//
//     mHandler.addCommand(&mCmdHelp);
//     // mHandler.addCommand(&mCmdPixel);
//     // mHandler.addCommand(&mCmdI2CDetect);
//
//     console_set_command_handler(&mHandler);
// }
//
// int32_t Application::run()
// {
//     // int32_t error = 0;
//     // initialize();
//
//     // conwaysSetDisplay(&mDisplay);
//     // multicore_launch_core1(conwaysRun);
//
//     // LOG_INFO("Conways Version: %s\n", CONWAYS_VERSION);
//     // LOG_INFO(" Common Version: %s\n", COMMON_VERSION);
//     // console_run();
//
//     return error;
// }
