#ifndef AT24C_H
#define AT24C_H

#include "hardware/i2c.h"

#define AT24CXXX_I2C_ADDRESS(A0, A1) 0x50 | (A1 & (0x1 << 2)) | (A0 & (0x1 << 1))
#define AT24CXXX_PAGE_SIZE  64
#define AT24CXXX_I2C_READ    0x1
#define AT24CXXX_I2C_WRITE   0x0

typedef struct at24cxxx_i2c_device_t {
    i2c_inst_t *bus;
    uint8_t address;
} at24cxxx_i2c_device;

int32_t at24cxxx_byte_write(at24cxxx_i2c_device *device, uint8_t *buffer, uint32_t buffer_length);
int32_t at24cxxx_byte_read(at24cxxx_i2c_device *device, uint8_t *buffer, uint32_t buffer_length);
int32_t at24cxxx_random_read(at24cxxx_i2c_device *device, uint16_t address, uint8_t *buffer, uint32_t buffer_length);

#endif // AT24C_H
