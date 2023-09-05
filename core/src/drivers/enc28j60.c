#include "core/logger.h"
#include "core/drivers/enc28j60.h"

#define OP_CODE_READ_CONTROL_REG    0
#define OP_CODE_READ_BUFFER_MEMORY  1
#define OP_CODE_WRITE_CONTROL_REG   2
#define OP_CODE_WRITE_BUFFER_MEMORY 3
#define OP_CODE_BIT_FIELD_SET       4
#define OP_CODE_BIT_FIELD_CLEAR     5
#define OP_CODE_SYSTEM_RESET        6

#define WS2812_COMMAND(OP, ARG) ((OP & 0x7) << 5) | (ARG & 0x1F)

int32_t enc28j60_spi_read(enc28j60_spi_device *device, uint8_t reg, uint8_t *buffer, size_t buffer_length)
{
    uint8_t cmd = WS2812_COMMAND(OP_CODE_READ_CONTROL_REG, reg);

    LOG_INFO("Command: 0x%02X\n", cmd);

    // Take chip select low
    gpio_put(device->cs, 0);
    // Write the opcode and control register
    spi_write_blocking(device->bus, &cmd, sizeof(cmd));
    // Read the response
    spi_read_blocking(device->bus, 0x00, buffer, buffer_length);
    // Take the chip select high
    gpio_put(device->cs, 1);

    return 0;
}

int32_t enc28j60_spi_write_control(enc28j60_spi_device *device, uint8_t reg, uint8_t data)
{
    uint8_t cmd[2] = {WS2812_COMMAND(OP_CODE_WRITE_CONTROL_REG, reg), data};

    LOG_INFO("Command: 0x%02X 0x%02X\n", cmd[0], cmd[1]);

    // Take chip select low
    gpio_put(device->cs, 0);

    // Write the opcode and control register
    spi_write_blocking(device->bus, cmd, sizeof(cmd));

    // Take the chip select high
    gpio_put(device->cs, 1);

    return 0;
}

int32_t enc28j60_spi_reset(enc28j60_spi_device *device)
{
    uint8_t cmd = WS2812_COMMAND(OP_CODE_SYSTEM_RESET, 0x1F);

    LOG_INFO("Command: 0x%02X\n", cmd);

    // Take chip select low
    gpio_put(device->cs, 0);

    // Write the opcode and control register
    spi_write_blocking(device->bus, &cmd, sizeof(cmd));

    // Take the chip select high
    gpio_put(device->cs, 1);

    return 0;
}
