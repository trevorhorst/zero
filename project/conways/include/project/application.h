#ifndef CONWAYS_APPLICATION_H
#define CONWAYS_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "common/drivers/ssd1306.h"

#include "project/game.h"

class Application
{
public:
    static const uint32_t pin_game_reset;
    static const uint32_t pin_game_speed;
    static const uint32_t pin_i2c1_sda;
    static const uint32_t pin_i2c1_scl;

    static const uint8_t ssd1306_display_addr;

    Application();

    void initialize();
    void initializeI2C();
    void initializeDisplay();

    int32_t run();

private:
    SSD1306 mDisplay;

};

#endif // CONWAYS_APPLICATION_H