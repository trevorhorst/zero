#include "core/logger.h"
#include "core/drivers/enc28j60.h"

int32_t enc28j60_initialize(enc28j60_spi_device *device)
{
    gpio_put(device->cs, 0);

    enc28j60_soft_reset(device);
    enc28j60_write_op(device, ENC28J60_WRITE_CTRL_REG, ECON1, 0x00);

    device->net->next_packet_ptr = RXSTART_INIT;

    LOG_INFO("Initialize Buffers\n");
    // Rx start
    enc28j60_write(device, ERXSTL, RXSTART_INIT & 0xFF);
    enc28j60_write(device, ERXSTH, RXSTART_INIT >> 8);
    // set receive pointer address
    enc28j60_write(device, ERXRDPTL, RXSTART_INIT & 0xFF);
    enc28j60_write(device, ERXRDPTH, RXSTART_INIT >> 8);
    // RX end
    enc28j60_write(device, ERXNDL, RXEND_INIT & 0xFF);
    enc28j60_write(device, ERXNDH, RXEND_INIT >> 8);
    // TX start
    enc28j60_write(device, ETXSTL, TXSTART_INIT & 0xFF);
    enc28j60_write(device, ETXSTH, TXSTART_INIT >> 8);
    // TX end
    enc28j60_write(device, ETXNDL, TXEND_INIT & 0xFF);
    enc28j60_write(device, ETXNDH, TXEND_INIT >> 8);

    LOG_INFO("Bank 1 stuff\n");
    // do bank 1 stuff, packet filter:
    // For broadcast packets we allow only ARP packtets
    // All other packets should be unicast only for our mac (MAADR)
    //
    // The pattern to match on is therefore
    // Type     ETH.DST
    // ARP      BROADCAST
    // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
    // in binary these poitions are:11 0000 0011 1111
    // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
    enc28j60_write(device, ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
    // enc28j60_write(device, EPMM0, 0x3f);
    // enc28j60_write(device, EPMM1, 0x30);
    // enc28j60_write(device, EPMCSL, 0xf9);
    // enc28j60_write(device, EPMCSH, 0xf7);

    LOG_INFO("Bank 2 stuff\n");
    //
    //
    // do bank 2 stuff
    // enable MAC receive
    enc28j60_write(device, MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
    // // bring MAC out of reset
    // enc28j60_write(device, MACON2, 0x00);
    // enable automatic padding to 60bytes and CRC operations
    enc28j60_write_op(device, ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
    // set inter-frame gap (non-back-to-back)
    enc28j60_write(device, MAIPGL, 0x12);
    enc28j60_write(device, MAIPGH, 0x0C);
    // set inter-frame gap (back-to-back)
    enc28j60_write(device, MABBIPG, 0x12);
    // Set the maximum packet size which the controller will accept
    // Do not send packets longer than MAX_FRAMELEN:
    enc28j60_write(device, MAMXFLL, MAX_FRAMELEN & 0xFF);
    enc28j60_write(device, MAMXFLH, MAX_FRAMELEN >> 8);

    LOG_INFO("Bank 3 stuff\n");
    // do bank 3 stuff
    // write MAC address
    // NOTE: MAC address in ENC28J60 is byte-backward
    enc28j60_set_mac(device);
    // enc28j60_write(device, MAADR5, macaddr[0]);
    // enc28j60_write(device, MAADR4, macaddr[1]);
    // enc28j60_write(device, MAADR3, macaddr[2]);
    // enc28j60_write(device, MAADR2, macaddr[3]);
    // enc28j60_write(device, MAADR1, macaddr[4]);
    // enc28j60_write(device, MAADR0, macaddr[5]);

    // LOG_INFO("Enable device\n");
    // // no loopback of transmitted frames
    // enc28j60_phy_write(device, PHCON2, PHCON2_HDLDIS);
    // // switch to bank 0
    // enc28j60_set_bank(device, ECON1);
    // // enable interrutps
    // enc28j60_write_op(device, ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
    // // enable packet reception
    // enc28j60_write_op(device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

    return 0;
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

void enc28j60_write_buffer(enc28j60_spi_device *device, uint16_t len, uint8_t *data)
{
    // Take chip select low
    gpio_put(device->cs, 0);

    // issue write command
    enc28j60_write_byte(device, ENC28J60_WRITE_BUF_MEM);

    if(device->debug){ printf("writing %d bytes to SPI\n", len); }

    // write byte by byte
    // for (int i = 0; i < len; i+=4) {
    // 	printf("byte %d = %02x %02x %02x %02x\n", i, data[i], data[i+1], data[i+2], data[i+3]);
    // }

    // for (int i = 0; i < len; i++) {
    // 	// printf("byte %d = %02x\n", i, data[i]);
    // 	if (spi_is_writable(spi_default) == 0) {
    // 		printf("SPI: NO SPACE AVAILABLE FOR WRITE\n");
    // 	}
    // 	spi_write_blocking(spi_default, &data[i], 1);
    // }
    spi_write_blocking(device->bus, data, len);

    // Take chip select low
    gpio_put(device->cs, 0);
}

void enc28j60_read_buffer(enc28j60_spi_device *device, uint16_t len, uint8_t *data)
{
    // Take chip select low
    gpio_put(device->cs, 0);

    // issue read command
    enc28j60_write_byte(device, ENC28J60_READ_BUF_MEM);

    spi_read_blocking(spi_default, 0, data, len);

    // Take chip select low
    gpio_put(device->cs, 0);
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
    while(enc28j60_read(device, (MISTAT)) & MISTAT_BUSY) {
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

void enc28j60_init(enc28j60_spi_device *device)
{
    gpio_put(device->cs, 0);

    enc28j60_soft_reset(device);

    device->net->next_packet_ptr = RXSTART_INIT;

    // Rx start
    enc28j60_write(device, ERXSTL, RXSTART_INIT & 0xFF);
    enc28j60_write(device, ERXSTH, RXSTART_INIT >> 8);
    // set receive pointer address
    enc28j60_write(device, ERXRDPTL, RXSTART_INIT & 0xFF);
    enc28j60_write(device, ERXRDPTH, RXSTART_INIT >> 8);
    // RX end
    enc28j60_write(device, ERXNDL, RXEND_INIT & 0xFF);
    enc28j60_write(device, ERXNDH, RXEND_INIT >> 8);
    // TX start
    enc28j60_write(device, ETXSTL, TXSTART_INIT & 0xFF);
    enc28j60_write(device, ETXSTH, TXSTART_INIT >> 8);
    // TX end
    enc28j60_write(device, ETXNDL, TXEND_INIT & 0xFF);
    enc28j60_write(device, ETXNDH, TXEND_INIT >> 8);

    // do bank 1 stuff, packet filter:
    // For broadcast packets we allow only ARP packtets
    // All other packets should be unicast only for our mac (MAADR)
    //
    // The pattern to match on is therefore
    // Type     ETH.DST
    // ARP      BROADCAST
    // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
    // in binary these poitions are:11 0000 0011 1111
    // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
    enc28j60_write(device, ERXFCON, ERXFCON_UCEN | ERXFCON_CRCEN | ERXFCON_PMEN);
    enc28j60_write(device, EPMM0, 0x3f);
    enc28j60_write(device, EPMM1, 0x30);
    enc28j60_write(device, EPMCSL, 0xf9);
    enc28j60_write(device, EPMCSH, 0xf7);

    //
    //
    // do bank 2 stuff
    // enable MAC receive
    enc28j60_write(device, MACON1, MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
    // // bring MAC out of reset
    // enc28j60_write(device, MACON2, 0x00);
    // enable automatic padding to 60bytes and CRC operations
    enc28j60_write_op(device, ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN);
    // set inter-frame gap (non-back-to-back)
    enc28j60_write(device, MAIPGL, 0x12);
    enc28j60_write(device, MAIPGH, 0x0C);
    // set inter-frame gap (back-to-back)
    enc28j60_write(device, MABBIPG, 0x12);
    // Set the maximum packet size which the controller will accept
    // Do not send packets longer than MAX_FRAMELEN:
    enc28j60_write(device, MAMXFLL, MAX_FRAMELEN & 0xFF);
    enc28j60_write(device, MAMXFLH, MAX_FRAMELEN >> 8);

    // do bank 3 stuff
    // write MAC address
    // NOTE: MAC address in ENC28J60 is byte-backward
    enc28j60_set_mac(device);
    // enc28j60_write(device, MAADR5, macaddr[0]);
    // enc28j60_write(device, MAADR4, macaddr[1]);
    // enc28j60_write(device, MAADR3, macaddr[2]);
    // enc28j60_write(device, MAADR2, macaddr[3]);
    // enc28j60_write(device, MAADR1, macaddr[4]);
    // enc28j60_write(device, MAADR0, macaddr[5]);

    // no loopback of transmitted frames
    enc28j60_phy_write(device, PHCON2, PHCON2_HDLDIS);
    // switch to bank 0
    enc28j60_set_bank(device, ECON1);
    // enable interrutps
    enc28j60_write_op(device, ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE | EIE_PKTIE);
    // enable packet reception
    enc28j60_write_op(device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

void enc28j60_hw_disable(enc28j60_spi_device *device)
{
    /* disable interrupts and packet reception */
    enc28j60_write_op(device, ENC28J60_WRITE_CTRL_REG, EIE, 0x00);
    enc28j60_write_op(device, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_RXEN);
    device->net->hw_enable = false;
}

void enc28j60_hw_enable(enc28j60_spi_device *device)
{
    /* enable interrupts */
    enc28j60_phy_write(device, PHIE, PHIE_PGEIE | PHIE_PLNKIE);

    enc28j60_write_op(device, ENC28J60_BIT_FIELD_CLR, EIR, EIR_DMAIF | EIR_LINKIF |
             EIR_TXIF | EIR_TXERIF | EIR_RXERIF | EIR_PKTIF);
    enc28j60_write_op(device, ENC28J60_WRITE_CTRL_REG, EIE, EIE_INTIE | EIE_PKTIE | EIE_LINKIE |
              EIE_TXIE | EIE_TXERIE | EIE_RXERIE);

    /* enable receive logic */
    enc28j60_write_op(device, ENC28J60_WRITE_CTRL_REG, ECON1, ECON1_RXEN);
    device->net->hw_enable = true;
}

void enc28j60_soft_reset(enc28j60_spi_device *device)
{
    // Errata workaround #1, CLKRDY check is unreliable, delay at least 1 ms instead
    enc28j60_write_op(device, ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
    sleep_ms(2);
}

// read the revision of the chip:
uint8_t enc28j60_get_rev(enc28j60_spi_device *device)
{
    return enc28j60_read(device, EREVID);
}

void enc28j60_set_mac(enc28j60_spi_device *device)
{
    enc28j60_write(device, MAADR5, device->net->dev_addr[0]);
    enc28j60_write(device, MAADR4, device->net->dev_addr[1]);
    enc28j60_write(device, MAADR3, device->net->dev_addr[2]);
    enc28j60_write(device, MAADR2, device->net->dev_addr[3]);
    enc28j60_write(device, MAADR1, device->net->dev_addr[4]);
    enc28j60_write(device, MAADR0, device->net->dev_addr[5]);
}

void enc28j60_packet_send(enc28j60_spi_device *device, uint16_t len, uint8_t *packet)
{
    // Set the write pointer to start of transmit buffer area
    enc28j60_write(device, EWRPTL, TXSTART_INIT & 0xFF);
    enc28j60_write(device, EWRPTH, TXSTART_INIT >> 8);

    // Set the TXND pointer to correspond to the packet size given
    enc28j60_write(device, ETXNDL, (TXSTART_INIT + len) & 0xFF);
    enc28j60_write(device, ETXNDH, (TXSTART_INIT + len) >> 8);
    // write per-packet control byte (0x00 means use macon3 settings)
    enc28j60_write_op(device, ENC28J60_WRITE_BUF_MEM, 0, 0x00);
    // copy the packet into the transmit buffer
    enc28j60_write_buffer(device, len, packet);

    // send the contents of the transmit buffer onto the network
    enc28j60_write_op(device, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
    // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
    // http://ww1.microchip.com/downloads/en/DeviceDoc/80349c.pdf
    if((enc28j60_read(device, EIR) & EIR_TXERIF)) {
        enc28j60_write_op(device, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
    }

    // status vector: TXND + 1;
    // uint16_t status = ((enc28j60Read(ETXNDH) << 8) | enc28j60Read(ETXNDL)) + 1;
}

uint16_t enc28j60_packet_receive(enc28j60_spi_device *device, uint16_t maxlen, uint8_t *packet)
{
    uint16_t rxstat;
    uint16_t len;
    // check if a packet has been received and buffered
    //if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
    // The above does not work. See Rev. B4 Silicon Errata point 6.
    if(enc28j60_read(device, EPKTCNT) == 0) {
        return (0);
    } else {
        LOG_INFO("Packet Received\n");
    }

    // Set the read pointer to the start of the received packet
    enc28j60_write(device, ERDPTL, (device->net->next_packet_ptr));
    enc28j60_write(device, ERDPTH, (device->net->next_packet_ptr) >> 8);
    // read the next packet pointer
    device->net->next_packet_ptr = enc28j60_read_op(device, ENC28J60_READ_BUF_MEM, 0);
    device->net->next_packet_ptr |= enc28j60_read_op(device, ENC28J60_READ_BUF_MEM, 0) << 8;
    LOG_INFO("Read next packet pointer: %lu\n", device->net->next_packet_ptr);
    // read the packet length (see datasheet page 43)
    len = enc28j60_read_op(device, ENC28J60_READ_BUF_MEM, 0);
    len |= enc28j60_read_op(device, ENC28J60_READ_BUF_MEM, 0) << 8;
    len -= 4; //remove the CRC count
    LOG_INFO("Packet length: %lu\n", len);
    // read the receive status (see datasheet page 43)
    rxstat = enc28j60_read_op(device, ENC28J60_READ_BUF_MEM, 0);
    rxstat |= enc28j60_read_op(device, ENC28J60_READ_BUF_MEM, 0) << 8;
    // limit retrieve length
    if (len > maxlen - 1) {
        len = maxlen - 1;
    }
    // check CRC and symbol errors (see datasheet page 44, table 7-3):
    // The ERXFCON.CRCEN is set by default. Normally we should not
    // need to check this.
    if ((rxstat & 0x80) == 0) {
        // invalid
        len = 0;
    } else {
        // copy the packet from the receive buffer
        enc28j60_read_buffer(device, len, packet);
    }
    // Move the RX read pointer to the start of the next received packet
    // This frees the memory we just read out
    enc28j60_write(device, ERXRDPTL, (device->net->next_packet_ptr));
    enc28j60_write(device, ERXRDPTH, (device->net->next_packet_ptr) >> 8);
    // decrement the packet counter indicate we are done with this packet
    enc28j60_write_op(device, ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    return (len);
}

void enc28j60_check_link_status(enc28j60_spi_device *device)
{
    uint16_t reg;
    int duplex;

    reg = enc28j60_phy_read(device, PHSTAT2);
    if(device->debug) { LOG_INFO("%s() PHSTAT1: %04x, PHSTAT2: %04x\n", __func__, enc28j60_phy_read(device, PHSTAT1), reg); }
    duplex = reg & PHSTAT2_DPXSTAT;

    if (reg & PHSTAT2_LSTAT) {
        // netif_carrier_on(ndev);
        LOG_INFO( "link up - %s\n", duplex ? "Full duplex" : "Half duplex");
    } else {
        LOG_INFO("link down\n");
    }
}
