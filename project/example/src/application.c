#include "core/logger.h"
#include "core/console/console.h"
#include "core/drivers/ws2812.h"

#include "project/application.h"

int32_t application_run()
{
    int32_t success = 0;
    stdio_init_all();

    sleep_ms(1000);

    // Initialize neopixel device information
    ws2812_device neopixel;
    neopixel.pin = 16;
    neopixel.length = 1;
    neopixel.pio = pio0;
    neopixel.sm = 0;
    neopixel.bytes[0] = WS2812_DATA_BYTE_NONE;
    neopixel.bytes[1] = WS2812_DATA_BYTE_GREEN;
    neopixel.bytes[2] = WS2812_DATA_BYTE_RED;
    neopixel.bytes[3] = WS2812_DATA_BYTE_BLUE;

    // Initialie the pixel to a blank state
    ws2812_initialize(&neopixel);
    ws2812_fill(&neopixel, WS2812_RGB(0, 0, 0));
    ws2812_show(&neopixel);

    LOG_INFO(" Example Version: %s\n", EXAMPLE_VERSION);
    LOG_INFO("  Common Version: %s\n", CORE_VERSION);

    // Initialize and launch the console on second core
    console_initialize();
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
