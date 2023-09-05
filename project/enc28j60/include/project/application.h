#ifndef EN28J60_APPLICATION_H
#define EN28J60_APPLICATION_H

#include <stdint.h>

#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/multicore.h"

#ifndef ENC28J60_VERSION
#define ENC28J60_VERSION "Not Found"
#endif // ENC28J60_VERSION

int32_t application_run();

#endif // ENC28J60_APPLICATION_H
