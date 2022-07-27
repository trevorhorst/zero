#ifndef EPAPER_APPLICATION_H
#define EPAPER_APPLICATION_H

#include <stdio.h>
#include "pico/stdlib.h"

#include "common/console/console.h"
#include "common/drivers/epaper.h"
#include "common/drivers/ws2812.h"
#include "common/logger.h"

#define UNIT_MHZ(x) x * 1000000

class Application
{
    static const uint32_t pin_display_cd;
    static const uint32_t pin_spi0_cs;
    static const uint32_t pin_spi0_sck;
    static const uint32_t pin_spi0_mosi;
    static const uint32_t pin_display_busy;
    static const uint32_t pin_display_reset;
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
    DrvEPaper mEPaper;
    WS2812 mNeopixel;
};

#endif //EPAPER_APPLICATION_H