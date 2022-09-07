#ifndef OLED_APPLICATION_H
#define OLED_APPLICATION_H

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"

#ifndef OLED_VERSION
#define OLED_VERSION "Not Found"
#endif // OLED_VERSION

int32_t application_run();

#endif // OLED_APPLICATION_H