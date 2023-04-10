#include "core/drivers/at24c.h"
#include "core/logger.h"

int32_t at24cxxx_byte_write(at24cxxx_i2c_device *device, uint8_t *buffer, uint32_t buffer_length)
{
    if(buffer_length >= AT24CXXX_PAGE_SIZE) {
        LOG_WARN("Write to device exceeds page size of %d bytes", AT24CXXX_PAGE_SIZE);
    }
    // return i2c_write_blocking(device->bus, (device->address | AT24CXXX_I2C_WRITE), buffer, buffer_length, false);
    return i2c_write_blocking(device->bus, device->address, buffer, buffer_length, false);
}

int32_t at24cxxx_byte_read(at24cxxx_i2c_device *device, uint8_t *buffer, uint32_t buffer_length)
{
    // return i2c_read_blocking(device->bus, (device->address | AT24CXXX_I2C_READ), buffer, buffer_length, false);
    return i2c_read_blocking(device->bus, device->address, buffer, buffer_length, false);
}

int32_t at24cxxx_random_read(at24cxxx_i2c_device *device, uint16_t address, uint8_t *buffer, uint32_t buffer_length)
{
    uint8_t address_buffer[2] = {((address >> 8) & 0xFF), ((address >> 0) & 0xFF)};
    i2c_write_blocking(device->bus, device->address, address_buffer, sizeof(address_buffer), false);
    // return i2c_read_blocking(device->bus, (device->address | AT24CXXX_I2C_READ), buffer, buffer_length, false);
    return i2c_read_blocking(device->bus, device->address, buffer, buffer_length, false);
}
