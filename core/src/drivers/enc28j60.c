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

int32_t enc28j20_initialize(enc28j60_spi_device *device)
{
    LOG_INFO("  Revision ID: %d\n", enc28j60_get_revision(device));

    bool full_duplex = true;

    // Receive buffer
    LOG_INFO("  Initialize RX Buffer\n");
    enc28j20_initialize_rx_fifo(device, WS2812_RXSTART_INIT, WS2812_RXEND_INIT);

    // Transmit buffer
    LOG_INFO("  Initialize TX Buffer\n");
    enc28j20_initialize_tx_fifo(device, WS2812_TXSTART_INIT, WS2812_TXEND_INIT);

    // Receive filters: (Unicast or Broadcast) AND crc valid
    LOG_INFO("  Initialize receive filters\n");
    enc28j60_set_bank(device, 1);
    enc28j60_spi_write_control(device, ERXFCON, (uint8_t)(ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_BCEN));

    // MAC Initialization
    LOG_INFO("  Initialize MAC\n");
    enc28j60_set_bank(device, 2);
    if(full_duplex) {
        enc28j60_spi_write_control(device, MACON3,
                    MACON3_PADCFG0 | MACON3_TXCRCEN |
                    MACON3_FRMLNEN | MACON3_FULDPX);
        /* set inter-frame gap (non-back-to-back) */
        enc28j60_spi_write_control(device, MAIPGL, 0x12);
        /* set inter-frame gap (back-to-back) */
        enc28j60_spi_write_control(device, MABBIPG, 0x15);
    } else {
        enc28j60_spi_write_control(device, MACON3,
                    MACON3_PADCFG0 | MACON3_TXCRCEN |
                    MACON3_FRMLNEN);
        enc28j60_spi_write_control(device, MACON4, 1 << 6);	/* DEFER bit */
        /* set inter-frame gap (non-back-to-back) */
        enc28j60_spi_write_control(device, MAIPGL, 0x12);
        enc28j60_spi_write_control(device, MAIPGH, 0x0C);
        /* set inter-frame gap (back-to-back) */
        enc28j60_spi_write_control(device, MABBIPG, 0x12);
    }
    enc28j60_spi_write_control(device, MAMXFLL, (uint8_t)(MAX_FRAMELEN >> 0));
    enc28j60_spi_write_control(device, MAMXFLL, (uint8_t)(MAX_FRAMELEN >> 8));

    // // PHY Initialization
    // LOG_INFO("  Initialize PHY\n");
    // if(full_duplex) {
    //     if(!enc28j60_spi_write_phy(device, PHCON1, PHCON1_PDPXMD)) {
    //         return 0;
    //     }
    //     if(!enc28j60_spi_write_phy(device, PHCON2, 0x00)) {
    //         return 0;
    //     }
    // } else {
    //     if(!enc28j60_spi_write_phy(device, PHCON1, 0x00)) {
    //         return 0;
    //     }
    //     if(!enc28j60_spi_write_phy(device, PHCON2, PHCON2_HDLDIS)) {
    //         return 0;
    //     }
    // }

    return 1;
}

void enc28j60_hw_disable(enc28j60_spi_device *device)
{
    /* disable interrupts and packet reception */
    enc28j60_spi_write_control(device, EIE, 0x00);
    enc28j60_spi_bitfield_clear(device, ECON1, ECON1_RXEN);
}

void enc28j60_hw_enable(enc28j60_spi_device *device)
{
    enc28j60_spi_write_phy(device, PHIE, (PHIE_PGEIE | PHIE_PLNKIE));
    enc28j60_spi_bitfield_clear(device, EIR, (EIR_DMAIF | EIR_LINKIF |
             EIR_TXIF | EIR_TXERIF | EIR_RXERIF | EIR_PKTIF));
    enc28j60_spi_write_control(device, EIE, (EIE_INTIE | EIE_PKTIE | EIE_LINKIE | EIE_TXIE | EIE_TXERIE | EIE_RXERIE));

    // Enable receive logic
    enc28j60_spi_bitfield_set(device, ECON1, ECON1_RXEN);
}

static uint16_t erxrdpt_workaround(uint16_t next_packet_ptr, uint16_t start, uint16_t end)
{
    uint16_t erxrdpt;

    if ((next_packet_ptr - 1 < start) || (next_packet_ptr - 1 > end))
        erxrdpt = end;
    else
        erxrdpt = next_packet_ptr - 1;

    return erxrdpt;
}

void enc28j20_initialize_rx_fifo(enc28j60_spi_device *device, uint16_t start, uint16_t end)
{
    if (start > 0x1FFF || end > 0x1FFF || start > end) {
        LOG_ERROR("(%d, %d) bad parameters\n", start, end);
        return;
    }

    // Configure bank 0
    enc28j60_set_bank(device, 0);

    // Write the start address
    enc28j60_spi_write_control(device, ERXSTL, (uint8_t)(start >> 0));
    enc28j60_spi_write_control(device, ERXSTH, (uint8_t)(start >> 8));

    // Write the receive pointer
    uint16_t erxrdpt = erxrdpt_workaround(start, start, end);
    enc28j60_spi_write_control(device, ERXRDPTL, (uint8_t)(erxrdpt >> 0));
    enc28j60_spi_write_control(device, ERXRDPTH, (uint8_t)(erxrdpt >> 8));

    // Write the end address
    enc28j60_spi_write_control(device, ERXNDL, (uint8_t)(end >> 0));
    enc28j60_spi_write_control(device, ERXNDH, (uint8_t)(end >> 8));
}

void enc28j20_initialize_tx_fifo(enc28j60_spi_device *device, uint16_t start, uint16_t end)
{
    if (start > 0x1FFF || end > 0x1FFF || start > end) {
        LOG_ERROR("(%d, %d) bad parameters\n", start, end);
        return;
    }

    // Configure bank 0
    enc28j60_set_bank(device, 0);

    // Write the start address
    enc28j60_spi_write_control(device, ETXSTL, (uint8_t)(start >> 0));
    enc28j60_spi_write_control(device, ETXSTH, (uint8_t)(start >> 8));

    // Write the end address
    enc28j60_spi_write_control(device, ETXNDL, (uint8_t)(end >> 0));
    enc28j60_spi_write_control(device, ETXNDH, (uint8_t)(end >> 8));
}

int32_t enc28j60_spi_read(enc28j60_spi_device *device, uint8_t reg, uint8_t *buffer, size_t buffer_length)
{
    uint8_t cmd = WS2812_COMMAND(OP_CODE_READ_CONTROL_REG, reg);

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

void enc28j60_spi_bitfield_clear(enc28j60_spi_device *device, uint8_t reg, uint8_t data)
{
    uint8_t cmd[2] = {WS2812_COMMAND(OP_CODE_BIT_FIELD_CLEAR, reg), data};

    LOG_INFO("Command: 0x%02X 0x%02X\n", cmd[0], cmd[1]);

    // Take chip select low
    gpio_put(device->cs, 0);

    // Write the opcode and control register
    spi_write_blocking(device->bus, cmd, sizeof(cmd));

    // Take the chip select high
    gpio_put(device->cs, 1);
}

void enc28j60_spi_bitfield_set(enc28j60_spi_device *device, uint8_t reg, uint8_t data)
{
    uint8_t cmd[2] = {WS2812_COMMAND(OP_CODE_BIT_FIELD_SET, reg), data};

    LOG_INFO("Command: 0x%02X 0x%02X\n", cmd[0], cmd[1]);

    // Take chip select low
    gpio_put(device->cs, 0);

    // Write the opcode and control register
    spi_write_blocking(device->bus, cmd, sizeof(cmd));

    // Take the chip select high
    gpio_put(device->cs, 1);
}

int16_t enc28j60_spi_read_phy(enc28j60_spi_device *device, uint8_t reg)
{
    enc28j60_set_bank(device, WS2812_BANK2);
    // Set the PHY register address
    enc28j60_spi_write_control(device, MIREGADR, reg);
    // Start the register read operation
    enc28j60_spi_write_control(device, MICMD, MICMD_MIIRD);


    enc28j60_set_bank(device, WS2812_BANK3);
    // Wait until the PHY read completes
    while(true) {
        uint8_t value = 0;
        enc28j60_spi_read(device, MISTAT, &value, sizeof(value));
        if((value & MISTAT_BUSY) == 0) {
            break;
        } else {
            sleep_ms(100);
        }
    }

    enc28j60_set_bank(device, WS2812_BANK2);
    // Stop the register read operation
    enc28j60_spi_write_control(device, MICMD, 0x00);

    uint8_t rh = 0;
    uint8_t rl = 0;

    // enc28j60_set_bank(device, 2);
    enc28j60_spi_read(device, MIRDL, &rl, sizeof(rl));
    LOG_INFO("READ PHY LO: %02X\n", rl);
    enc28j60_spi_read(device, MIRDH, &rh, sizeof(rh));
    LOG_INFO("READ PHY HI: %02X\n", rh);

    uint16_t value = ((uint16_t)rh << 8) | (uint16_t)rl;
    return value;
}

int32_t enc28j60_spi_write_phy(enc28j60_spi_device *device, uint8_t reg, uint16_t data)
{
    enc28j60_set_bank(device, WS2812_BANK2);
    enc28j60_spi_write_control(device, MIREGADR, reg);
    enc28j60_spi_write_control(device, MIWRL, (uint8_t)(data >> 0));
    enc28j60_spi_write_control(device, MIWRH, (uint8_t)(data >> 8));

    enc28j60_set_bank(device, WS2812_BANK2);
    while(true) {
        uint8_t value = 0;
        enc28j60_spi_read(device, MISTAT, &value, sizeof(value));
        if((value & MISTAT_BUSY) == 0) {
            break;
        } else {
            sleep_ms(100);
        }
    }

    return 1;
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

uint8_t enc28j60_get_revision(enc28j60_spi_device *device)
{
    uint8_t revision = 0;
    enc28j60_set_bank(device, WS2812_BANK3);
    enc28j60_spi_read(device, EREVID, &revision, sizeof(revision));
    return revision;
}

int32_t enc28j60_set_bank(enc28j60_spi_device *device, uint8_t bank)
{
    uint32_t error = 0;

    enc28j60_spi_write_control(device, ECON1, bank);

    return error;
}

int32_t enc28j60_set_mac(enc28j60_spi_device *device, uint8_t *address)
{
    uint32_t error = 0;

    enc28j60_set_bank(device, WS2812_BANK3);

    enc28j60_spi_write_control(device, MAADR6, address[0]);
    enc28j60_spi_write_control(device, MAADR5, address[1]);
    enc28j60_spi_write_control(device, MAADR4, address[2]);
    enc28j60_spi_write_control(device, MAADR3, address[3]);
    enc28j60_spi_write_control(device, MAADR2, address[4]);
    enc28j60_spi_write_control(device, MAADR1, address[5]);

    return error;
}

void enc28j60_check_link_status(enc28j60_spi_device *device)
{
    uint16_t reg = enc28j60_spi_read_phy(device, PHSTAT2);

    bool duplex = reg & PHSTAT2_DPXSTAT;

    if(reg & PHSTAT2_LSTAT) {
        LOG_INFO("link up - %s\n", duplex ? "Full duplex" : "Half duplex");
    } else {
        LOG_INFO("link down\n");
    }
}
