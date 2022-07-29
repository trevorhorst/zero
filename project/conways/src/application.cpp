#include "project/application.h"

#define I2C_BUS_SPEED_KHZ(x) x * 1000

// Debounce control
static uint32_t debounce_reset_button = to_ms_since_boot(get_absolute_time());
static uint32_t debounce_speed_button = to_ms_since_boot(get_absolute_time());
static const uint32_t debounce_delay_time = 500; // Delay for every push button may vary

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    // gpio_event_string(event_str, events);
    printf("GPIO Reset Event: %d\n", gpio);
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());

    if(gpio == Application::pin_game_reset) {
        if((currentTime - debounce_reset_button) > debounce_delay_time) {
            debounce_reset_button = currentTime;
            conwaysSetReset();
        }
    } else if(gpio == Application::pin_game_speed) {
        if((currentTime - debounce_speed_button) > debounce_delay_time) {
            debounce_speed_button = currentTime;
            conwaysStepSpeed();
        }
    }   

}

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

const uint32_t Application::pin_game_reset  = 2;
const uint32_t Application::pin_game_speed  = 3;
const uint32_t Application::pin_i2c1_sda     = 26;
const uint32_t Application::pin_i2c1_scl     = 27;

const uint8_t Application::ssd1306_display_addr = 0x3D;

Application::Application()
    : mDisplay(i2c1, ssd1306_display_addr)
{

}

void Application::initialize()
{
    stdio_init_all();

    // Give serial some time to come up
    sleep_ms(1000);

    srand(to_ms_since_boot(get_absolute_time()));

    // gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(pin_game_reset, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(pin_game_speed, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    initializeI2C();
    initializeDisplay();
}

void Application::initializeI2C()
{
    LOG_INFO("Initializing I2C...\n");
    i2c_init(i2c1, I2C_BUS_SPEED_KHZ(1000));
    gpio_set_function(pin_i2c1_sda, GPIO_FUNC_I2C);
    gpio_set_function(pin_i2c1_scl, GPIO_FUNC_I2C);
    gpio_pull_up(pin_i2c1_sda);
    gpio_pull_up(pin_i2c1_scl);

    // i2c_bus_scan(i2c1);
}
 
void Application::initializeDisplay()
{
    LOG_INFO("Initializing display...\n");
    LOG_INFO("Display @ 0x%02X\n", ssd1306_display_addr);
    mDisplay.initialize();

    // Flash display so we know it's alive
    mDisplay.fill_screen(0xFF);
    sleep_ms(500);
    mDisplay.fill_screen(0x00);
}

int32_t Application::run()
{
    int32_t error = 0;
    initialize();
    
    conwaysSetDisplay(&mDisplay);
    multicore_launch_core1(conwaysRun);
    
    LOG_INFO("Conways Version: %s\n", CONWAYS_VERSION);
    LOG_INFO(" Common Version: %s\n", COMMON_VERSION);
    console_run();

    return error;
}