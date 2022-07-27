#ifndef EPAPER_APPLICATION_H
#define EPAPER_APPLICATION_H

#include <stdio.h>
#include "pico/stdlib.h"

#include "common/console/console.h"
#include "common/logger.h"

class Application
{
public:
    Application();
    int32_t run();
};

#endif //EPAPER_APPLICATION_H