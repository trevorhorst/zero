#include "core/drivers/sht3xdis.h"

/**
 * @brief Calculate the CRC-8 checksum a received data from the chip
 * 
 * @param bytes 
 * @param nbytes 
 * @param polynomial 
 * @return uint32_t 
 */
uint32_t calculate_checksum(uint8_t *bytes, uint32_t nbytes)
{
    // Polynomial used by the sensor (x^8 + x^5 + x^4 + x^1)
    uint32_t polynomial = 0x131;

    // Initialize CRC register to 0
    uint32_t crc = 0;

    for(uint32_t byte = 0; byte < nbytes; byte++)
    {
        crc ^= bytes[byte];
        for(uint32_t bit = 8; bit > 0; --bit) {
            if(crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            } else { 
                crc = (crc << 1); 
            }
        }
    }

    return crc;
}


void sht3xdis_write(sht3xdis_i2c_device *device, uint8_t *buffer, uint32_t buffer_length)
{
    i2c_write_blocking(device->bus, device->address, buffer, buffer_length, false);
}

void sht3xdis_read(sht3xdis_i2c_device *device, uint8_t *buffer, uint32_t buffer_length)
{
    i2c_read_blocking(device->bus, device->address, buffer, buffer_length, false);
}

float sht3xdis_convert_raw_to_relative_humidity(uint16_t rawrh)
{
    float rh = (100. * ((float)rawrh / 65535.));
    return rh;
}

float sht3xdis_convert_raw_to_celsius(uint16_t temp)
{
    float temperature = -45. + (175. * ((float)temp / 65535.));
    return temperature;
}

float sht3xdis_convert_raw_to_farenheit(uint16_t temp)
{
    float temperature = -49. + (315. * ((float)temp / 65535.));
    return temperature;
}

sht3xdis_measurement sht3xdis_singleshot_measurement(sht3xdis_i2c_device *device, 
    enum sht3xdis_repeatability repeat, bool clock_stretch)
{
    uint8_t command[2] = {0x00, 0x00};
    command[0] = clock_stretch ? 0x2C : 0x24;
    switch(repeat) {
        case(SHT3XDIS_REPEATABILITY_HIGH):
            command[1] = clock_stretch ? 0x06 : 0x00;
            break;
        case(SHT3XDIS_REPEATABILITY_MEDIUM):
            command[1] = clock_stretch ? 0x0D : 0x0B;
            break;
        case(SHT3XDIS_REPEATABILITY_LOW):
            command[1] = clock_stretch ? 0x10 : 0x16;
            break;
    }
    sht3xdis_write(device, command, sizeof(command));
    sleep_ms(1000);
    uint8_t response[6];
    sht3xdis_read(device, response, sizeof(response));

    sht3xdis_measurement measurement;
    measurement.raw_temperature         = (response[0] << 8) | response[1];
    measurement.raw_relative_humidity   = (response[3] << 8) | response[4];

    return measurement;
}