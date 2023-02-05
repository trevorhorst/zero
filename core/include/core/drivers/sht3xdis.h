#ifndef SHT3XDIS_H
#define SHT3XDIS_H

#include "hardware/i2c.h"

// ADDR (pin 2) connected to ground
#define SHT3XDIX_I2C_ADDRESS_A  0x44
// ADDR (pin 2) connected to VDD
#define SHT3XDIX_I2C_ADDRESS_B  0x45

typedef struct sht3xdis_i2c_device_t {
    i2c_inst_t *bus;
    uint8_t address;
} sht3xdis_i2c_device;

/// @note Will get around to using these eventually... - TWH 02/05/2023
// typedef struct sht3xdis_raw_data_t {
//     uint8_t buffer[2];
//     uint8_t checksum;
// } sht3xdis_raw_data;
// 
// typedef union sht3xdis_raw_buffer_t {
//     sht3xdis_raw_data raw_data;
//     uint8_t raw_buffer[3];
// } sht3xdis_raw_buffer;

typedef struct sht3xdis_measurement_t {
    int16_t raw_temperature;
    int16_t raw_relative_humidity;
} sht3xdis_measurement;

enum sht3xdis_repeatability {
    SHT3XDIS_REPEATABILITY_HIGH     = 0,
    SHT3XDIS_REPEATABILITY_MEDIUM   = 1,
    SHT3XDIS_REPEATABILITY_LOW      = 2
};

void sht3xdis_write(sht3xdis_i2c_device *device, uint8_t *buffer, uint32_t buffer_length);
void sht3xdis_read(sht3xdis_i2c_device *device, uint8_t *buffer, uint32_t buffer_length);
float sht3xdis_convert_raw_to_relative_humidity(uint16_t rawrh);
float sht3xdis_convert_raw_to_celsius(uint16_t temp);
float sht3xdis_convert_raw_to_farenheit(uint16_t temp);
sht3xdis_measurement sht3xdis_singleshot_measurement(sht3xdis_i2c_device *device, 
    enum sht3xdis_repeatability repeat, bool clock_stretch);

#endif // SHT3XDIS_H