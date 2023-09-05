#include <malloc.h>

#include "hardware/pio.h"

#include "core/drivers/ws2812.h"

void ws2812_initialize(ws2812_device *device)
{
    device->data = (uint32_t*)malloc(sizeof(uint32_t) * device->length);

    // Configure the PIO program
    uint offset = pio_add_program(device->pio, &ws2812_program);
    ws2812_program_init(device->pio, device->sm, offset, device->pin, 800000, true);
}

void ws2812_uninitialize(ws2812_device *device)
{
    free(device->data);
}

uint32_t ws2812_convert_data(ws2812_device *device, uint32_t rgbw) {
    uint32_t result = 0;
    for (uint b = 0; b < 4; b++) {
        switch (device->bytes[b]) {
            case WS2812_DATA_BYTE_RED:
                result |= (rgbw & 0xFF);
                break;
            case WS2812_DATA_BYTE_GREEN:
                result |= (rgbw & 0xFF00) >> 8;
                break;
            case WS2812_DATA_BYTE_BLUE:
                result |= (rgbw & 0xFF0000) >> 16;
                break;
            case WS2812_DATA_BYTE_WHITE:
                result |= (rgbw & 0xFF000000) >> 24;
                break;
        }
        result <<= 8;
    }
    return result;
}

void ws2812_fill(ws2812_device *device, uint32_t color) {
    uint32_t first = 0;
    uint32_t count = device->length;
    uint last = (first + count);
    if (last > device->length) {
        last = device->length;
    }
    color = ws2812_convert_data(device, color);
    for (uint i = first; i < last; i++) {
        device->data[i] = color;
    }
}

void ws2812_show(ws2812_device *device) {
    #ifdef DEBUG
    for (uint i = 0; i < length; i++) {
        printf("WS2812 / Put data: %08X\n", data[i]);
    }
    #endif
    for(uint i = 0; i < device->length; i++) {
        pio_sm_put_blocking(device->pio, device->sm, device->data[i]);
    }
}
