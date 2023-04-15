#ifndef ADXL345_H
#define ADXL345_H

#include "hardware/i2c.h"
#include "core/logger.h"

#define ADXL345_I2C_ADDRESS(SD0)    SD0 ? 0x1D : 0x53

// Register map
#define ADXL345_REG_DEVID       0x00
#define ADXL345_REG_THRESH_TAP  0x1D
#define ADXL345_REG_POWER_CTL   0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0      0x32
#define ADXL345_REG_DATAX1      0x33
#define ADXL345_REG_DATAY0      0x34
#define ADXL345_REG_DATAY1      0x35
#define ADXL345_REG_DATAZ0      0x36
#define ADXL345_REG_DATAZ1      0x37

#define ADXL345_MEASUREMENT

typedef struct adxl345_i2c_device_t {
    i2c_inst_t *bus;
    uint8_t address;
} adxl345_i2c_device;

typedef union adxl345_data_format_t {
    struct {
        uint8_t range : 2;
        uint8_t justify : 1;
        uint8_t full_res : 1;
        uint8_t reserved : 1;
        uint8_t int_invert : 1;
        uint8_t spi : 1;
        uint8_t self_test : 1;
    } f;
    uint8_t value;
} adxl345_data_format;

typedef union adxl345_power_ctl_t {
    struct {
        uint8_t wakeup : 2;
        uint8_t sleep : 1;
        uint8_t measure : 1;
        uint8_t auto_sleep: 1;
        uint8_t link : 1;
        uint8_t reserved : 2;
    } f;
    uint8_t value;
} adxl345_power_ctl;

typedef union adxl345_data_t {
    struct {
        uint8_t datax[2];
        uint8_t datay[2];
        uint8_t dataz[2];
    } f;
    uint8_t value[6];
} adxl345_data;

#define ADXL345_RANGE_2G    0b00;
#define ADXL345_RANGE_4G    0b01;
#define ADXL345_RANGE_8G    0b10;
#define ADXL345_RANGE_16G   0b11;

int32_t adxl345_i2c_write_byte(adxl345_i2c_device *device, uint8_t address, uint8_t byte);
int32_t adxl345_i2c_read(adxl345_i2c_device *device, uint8_t address, uint8_t *buffer, uint32_t buffer_length);
int32_t adxl345_i2c_set_data_format(adxl345_i2c_device *device, adxl345_data_format *value);
int32_t adxl345_i2c_set_power_ctl(adxl345_i2c_device *device, const adxl345_power_ctl *value);

int32_t adxl345_i2c_get_thresh_tap(adxl345_i2c_device *device);
int32_t adxl345_i2c_get_data(adxl345_i2c_device *device, adxl345_data *data);

#endif // ADXL345_H