#ifndef EPAPER_APPLICATION_H
#define EPAPER_APPLICATION_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "common/command/command_help.h"
#include "common/console/console.h"
#include "common/drivers/epaper.h"
#include "common/drivers/ws2812.h"
#include "common/logger.h"
#include "common/version.h"

#include "project/facts.h"
#include "project/draw/draw.h"

#define UNIT_MHZ(x) x * 1000000

#ifndef EPAPER_VERSION
#define EPAPER_VERSION "Not Found"
#endif

class Application
{
    static const uint32_t neopixel_num_leds;
    static const uint32_t neopixel_max_brightness;
public:
    static const uint32_t pin_display_cd;
    static const uint32_t pin_spi0_cs;
    static const uint32_t pin_spi0_sck;
    static const uint32_t pin_spi0_mosi;
    static const uint32_t pin_display_busy;
    static const uint32_t pin_display_reset;
    static const uint32_t pin_generate_fact;
    static const uint32_t pin_neopixel;

    /**
     * @brief Construct a new Application object
     */
    Application();

    /*
     * @brief Initializes the various components for the application
     */
    void initialize();

    void initializeDrawing();

    void drawFact();

    /**
     * @brief Main application thread
     * 
     * @return int32_t Exit status of the application
     */
    int32_t run();

private:
    UBYTE *mImage;

    DrvEPaper mEPaper;
    WS2812 mNeopixel;

    CommandHandler mHandler;
    CommandHelp mCmdHelp;
};

#endif //EPAPER_APPLICATION_H