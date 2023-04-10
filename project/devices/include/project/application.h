#ifndef DEVICES_APPLICATION_H
#define DEVICES_APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include "pico/multicore.h"
#include "pico/stdlib.h"

#ifndef DEVICES_VERSION
#define DEVICES_VERSION "Not Found"
#endif // DEVICES_VERSION

int32_t application_run();

#endif // DEVICES_APPLICATION_H