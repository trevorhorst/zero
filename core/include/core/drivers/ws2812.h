#ifndef WS2812_H
#define WS2812_H

#include <stdint.h>

#include "pico/types.h"
#include "core/drivers/pio/ws2812.pio.h"

#define WS2812_DATA_BYTE_NONE   0
#define WS2812_DATA_BYTE_RED    1
#define WS2812_DATA_BYTE_GREEN  2
#define WS2812_DATA_BYTE_BLUE   3
#define WS2812_DATA_BYTE_WHITE  4

#define WS2812_DATA_FORMAT_RGB  0

#define WS2812_RGB(red, green, blue) \
    (uint32_t)(blue) << 16 | (uint32_t)(green) << 8 | (uint32_t)(red)

#define WS2812_RGBW(red, green, blue, white) \
    (uint32_t)(white) << 24 | (uint32_t)(blue) << 16 | (uint32_t)(green) << 8 | (uint32_t)(red)

typedef struct ws2812_device_t {
    uint32_t pin;
    uint32_t length;
    PIO pio;
    uint32_t sm;
    uint8_t bytes[4];
    uint32_t *data;
} ws2812_device;

void ws2812_initialize(ws2812_device *device);
void ws2812_uninitialize(ws2812_device *device);
uint32_t ws2812_convert_data(ws2812_device *device, uint32_t rgbw);
void ws2812_fill(ws2812_device *device, uint32_t color);
void ws2812_show(ws2812_device *device);

#endif // WS812_H
