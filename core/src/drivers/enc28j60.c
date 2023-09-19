#include "core/logger.h"
#include "core/drivers/enc28j60.h"

int32_t enc28j20_initialize(enc28j60_spi_device *device)
{

}

void enc28j60_write_byte(enc28j60_spi_device *device, uint8_t data)
{
    spi_write_blocking(device->bus, &data, sizeof(data));
}

uint8_t enc28j60_read_byte(enc28j60_spi_device *device)
{
    uint8_t data = 0;
    spi_read_blocking(device->bus, 0x00, &data, sizeof(data));
    return data;
}

void enc28j60_write_op(enc28j60_spi_device *device, uint8_t op, uint8_t address, uint8_t data)
{
    // Take chip select low
    gpio_put(device->cs, 0);

    // Issue write command
    enc28j60_write_byte(device, (op | (address & ADDR_MASK)));

    // write data
    enc28j60_write_byte(device, data);

    // Take chip select low
    gpio_put(device->cs, 1);

    if(device->debug){ LOG_INFO("[%02X] <-- %02X\n", (address & ADDR_MASK), data); }
}

uint8_t enc28j60_read_op(enc28j60_spi_device *device, uint8_t op, uint8_t address)
{
    // Take chip select low
    gpio_put(device->cs, 0);

    // issue read command
    enc28j60_write_byte(device, (op | (address & ADDR_MASK)));

    // read data
    uint8_t data = enc28j60_read_byte(device);

    // do dummy read if needed (for mac and mii, see datasheet page 29)
    if (address & 0x80) {
        data = enc28j60_read_byte(device);
    }

    // Take chip select low
    gpio_put(device->cs, 1);

    if(device->debug){ LOG_INFO("[%02X] --> %02X\n", (address & ADDR_MASK), data); }

    return data;
}

void enc28j60_set_bank(enc28j60_spi_device *device, uint8_t address)
{
    // set the bank
    enc28j60_write_op(device, ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1 | ECON1_BSEL0));
    enc28j60_write_op(device, ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK) >> 5);
}

uint8_t enc28j60_read(enc28j60_spi_device *device, uint8_t address)
{
    // set the bank
    enc28j60_set_bank(device, address);
    // do the read
    return enc28j60_read_op(device, ENC28J60_READ_CTRL_REG, address);
}

void enc28j60_write(enc28j60_spi_device *device, uint8_t address, uint8_t data)
{
    // set the bank
    enc28j60_set_bank(device, address);
    // do the write
    enc28j60_write_op(device, ENC28J60_WRITE_CTRL_REG, address, data);
}


void enc28j60_phy_write(enc28j60_spi_device *device, uint8_t address, uint16_t data)
{
    // set the PHY register address
    enc28j60_write(device, MIREGADR, address);
    // write the PHY data
    enc28j60_write(device, MIWRL, data);
    enc28j60_write(device, MIWRH, data >> 8);
    // wait until the PHY write completes
    while(enc28j60_read(device, (MISTAT & MISTAT_BUSY))) {
        sleep_ms(15);
    }
}

void enc28j60_wait_phy_ready(enc28j60_spi_device *device)
{
    while(true) {
        uint8_t value = enc28j60_read(device, MISTAT);
        if((value & MISTAT_BUSY) == 0) {
            break;
        } else {
            sleep_ms(100);
        }
    }
}

uint16_t enc28j60_phy_read(enc28j60_spi_device *device, uint8_t address)
{
    // Set the PHY register address
    enc28j60_write(device, MIREGADR, address);
    // Start the register read operation
    enc28j60_write(device, MICMD, MICMD_MIIRD);
    // Wait until the PHY read completes
    enc28j60_wait_phy_ready(device);
    // Quit reading
    enc28j60_write(device, MICMD, 0x00);

    uint16_t rl = enc28j60_read(device, MIRDL);
    uint16_t rh = enc28j60_read(device, MIRDH);

    return (rh << 8) | rl;
}

// read the revision of the chip:
uint8_t enc28j60_get_rev(enc28j60_spi_device *device)
{
    return enc28j60_read(device, EREVID);
}
