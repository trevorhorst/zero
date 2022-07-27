#ifndef EPAPER_APPLICATION_H
#define EPAPER_APPLICATION_H

#include <stdio.h>
#include "pico/stdlib.h"

#include "common/console/console.h"
#include "common/drivers/epaper.h"
#include "common/drivers/ws2812.h"
#include "common/logger.h"

class Application
{
    static const uint32_t pin_neopixel;

    static const uint32_t neopixel_num_leds;
    static const uint32_t neopixel_max_brightness;
public:
    Application();

    /**
     * @brief Initializes the various components for the application
     */
    void initialize();

    /**
     * @brief Main application thread
     * 
     * @return int32_t Exit status of the application
     */
    int32_t run();

private:
    // DrvEPaper mEPaper;
    WS2812 mNeopixel;
};

#endif //EPAPER_APPLICATION_H