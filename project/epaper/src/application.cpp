#include "project/application.h"

const uint32_t Application::pin_neopixel    = 16;

const uint32_t Application::neopixel_num_leds       = 1;
const uint32_t Application::neopixel_max_brightness = 25;

Application::Application() :
    mNeopixel(
        pin_neopixel,
        neopixel_num_leds,
        pio0,
        0,
        WS2812::DataFormat::FORMAT_GRB
    )
{
}

/**
 * @brief Initializes the various components required for operation
 */
void Application::initialize()
{
    stdio_init_all();

    // Just configure debug to show all messages for now
    log_set_level(LOG_TRACE);

    // Display green LED
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