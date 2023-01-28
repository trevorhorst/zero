#ifndef DEWPOINT_APPLICATION_H
#define DEWPOINT_APPLICATION_H

#include <stdio.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/time.h"

#ifndef DEWPOINT_VERSION
#define DEWPOINT_VERSION "Not Found"
#endif // DEWPOINT_VERSION

int32_t application_run();

#endif // DEWPOINT_APPLICATION_H