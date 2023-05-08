#include "core/drivers/adxl345.h"

int32_t adxl345_i2c_write_byte(adxl345_i2c_device *device, uint8_t address, uint8_t byte)
{
    uint8_t buffer[2] = {address, byte};
    return i2c_write_blocking(device->bus, device->address, buffer, sizeof(buffer), false);
}

int32_t adxl345_i2c_read(adxl345_i2c_device *device, uint8_t address, uint8_t *buffer, uint32_t buffer_length)
{
    // uint8_t address_buffer[2] = {((address >> 8) & 0xFF), ((address >> 0) & 0xFF)};
    i2c_write_blocking(device->bus, device->address, &address, sizeof(address), false);
    return i2c_read_blocking(device->bus, device->address, buffer, buffer_length, false);
}

uint8_t adxl345_i2c_get_devid(adxl345_i2c_device *device)
{
    uint8_t devid = 0;
    adxl345_i2c_read(device, ADXL345_REG_DEVID, &devid, sizeof(devid));
    return devid;
}

int32_t adxl345_i2c_get_thresh_tap(adxl345_i2c_device *device)
{
    uint8_t buffer[1];
    int32_t r = adxl345_i2c_read(device, ADXL345_REG_THRESH_TAP, buffer, sizeof(buffer));
    if(r == sizeof(buffer)) {
        for(int32_t i = 0; i < sizeof(buffer); i++) {
            LOG_INFO("0x%02X\n", buffer[i]);
        }
    } else {
        LOG_INFO("Read Unsuccessful\n");
    }
}

int32_t adxl345_i2c_get_data(adxl345_i2c_device *device, adxl345_data *data)
{
    int32_t error = 0;
    
    if(data) {
        int32_t r = adxl345_i2c_read(device, ADXL345_REG_DATAX0, data->value, sizeof(data->value));
        if(r != sizeof(data->value)) {
            LOG_ERROR("Read unsuccessful\n");
        }
    } else {
        error = -1;
    }

    return error;
}

int32_t adxl345_i2c_set_data_format(adxl345_i2c_device *device, adxl345_data_format *value)
{
    adxl345_i2c_write_byte(device, ADXL345_REG_DATA_FORMAT, value->value);
}

int32_t adxl345_i2c_set_power_ctl(adxl345_i2c_device *device, const adxl345_power_ctl *value)
{
    LOG_INFO("Power Ctl Value: 0x%02X\n", value->value);
    adxl345_i2c_write_byte(device, ADXL345_REG_POWER_CTL, value->value);
}