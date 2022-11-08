#ifndef CONWAYS_APPLICATION_H
#define CONWAYS_APPLICATION_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "hardware/spi.h"

// #include "common/command/command_handler.h"
// #include "common/command/command_help.h"
// #include "common/console/console.h"
// #include "common/drivers/ssd1306.h"
// #include "common/logger.h"
//
// #include "project/game.h"

#ifndef CONWAYS_VERSION
#define CONWAYS_VERSION "Not Found"
#endif // CONWAYS_VERSION

int32_t application_run();

#endif // CONWAYS_APPLICATION_H
