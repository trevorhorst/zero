#ifndef EXAMPLE_APPLICATION_H
#define EXAMPLE_APPLICATION_H

#include <stdint.h>

#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/multicore.h"

#ifndef EXAMPLE_VERSION
#define EXAMPLE_VERSION "Not Found"
#endif // EXAMPLE_VERSION

int32_t application_run();

#endif // EXAMPLE_APPLICATION_H
