#ifndef ENC28J60_H
#define ENC28J60_H

#include <stdint.h>

#include "hardware/spi.h"
#include "hardware/gpio.h"

// Bank 0 Registers
#define ERDPTL     _u(0x00)
#define ERDPTH     _u(0x01)
#define EWRPTL     _u(0x02)
#define EWRPTH     _u(0x03)
#define ETXSTL     _u(0x04)
#define ETXSTH     _u(0x05)
#define ETXNDL     _u(0x06)
#define ETXNDH     _u(0x07)
#define ERXSTL     _u(0x08)
#define ERXSTH     _u(0x09)
#define ERXNDL     _u(0x0A)
#define ERXNDH     _u(0x0B)
#define ERXRDPTL   _u(0x0C)
#define ERXRDPTH   _u(0x0D)
#define ERXWRPTL   _u(0x0E)
#define ERXWRPTH   _u(0x0F)
#define EDMASTL    _u(0x10)
#define EDMASTH    _u(0x11)
#define EDMANDL    _u(0x12)
#define EDMANDH    _u(0x13)
#define EDMADSTL   _u(0x14)
#define EDMADSTH   _u(0x15)
#define EDMACSL    _u(0x16)
#define EDMACSH    _u(0x17)
// #define —          _u(0x18)
// #define —          _u(0x19)
#define Reserved   _u(0x1A)
#define EIE        _u(0x1B)
#define EIR        _u(0x1C)
#define ESTAT      _u(0x1D)
#define ECON2      _u(0x1E)
#define ECON1      _u(0x1F)

// Bank 1 Registers
#define EHT0       _u(0x00)
#define EHT1       _u(0x01)
#define EHT2       _u(0x02)
#define EHT3       _u(0x03)
#define EHT4       _u(0x04)
#define EHT5       _u(0x05)
#define EHT6       _u(0x06)
#define EHT7       _u(0x07)
#define EPMM0      _u(0x08)
#define EPMM1      _u(0x09)
#define EPMM2      _u(0x0A)
#define EPMM3      _u(0x0B)
#define EPMM4      _u(0x0C)
#define EPMM5      _u(0x0D)
#define EPMM6      _u(0x0E)
#define EPMM7      _u(0x0F)
#define EPMCSL     _u(0x10)
#define EPMCSH     _u(0x11)
// #define —          _u(0x12)
// #define —          _u(0x13)
#define EPMOL      _u(0x14)
#define EPMOH      _u(0x15)
// #define Reserved   _u(0x16)
// #define Reserved   _u(0x17)
#define ERXFCON    _u(0x18)
#define EPKTCNT    _u(0x19)
// #define Reserved   _u(0x1A)
#define EIE        _u(0x1B)
#define EIR        _u(0x1C)
#define ESTAT      _u(0x1D)
#define ECON2      _u(0x1E)
#define ECON1      _u(0x1F)

// Bank 2 Registers
#define MACON1    _u(0x00)
// #define Reserved  _u(0x01)
#define MACON3    _u(0x02)
#define MACON4    _u(0x03)
#define MABBIPG   _u(0x04)
// #define —         _u(0x05)
#define MAIPGL    _u(0x06)
#define MAIPGH    _u(0x07)
#define MACLCON1  _u(0x08)
#define MACLCON2  _u(0x09)
#define MAMXFLL   _u(0x0A)
#define MAMXFLH   _u(0x0B)
// #define Reserved  _u(0x0C)
// #define Reserved  _u(0x0D)
// #define Reserved  _u(0x0E)
// #define —         _u(0x0F)
// #define Reserved  _u(0x10)
// #define Reserved  _u(0x11)
#define MICMD     _u(0x12)
// #define —         _u(0x13)
#define MIREGADR  _u(0x14)
// #define Reserved  _u(0x15)
#define MIWRL     _u(0x16)
#define MIWRH     _u(0x17)
#define MIRDL     _u(0x18)
#define MIRDH     _u(0x19)
// #define Reserved  _u(0x1A)
#define EIE       _u(0x1B)
#define EIR       _u(0x1C)
#define ESTAT     _u(0x1D)
#define ECON2     _u(0x1E)
#define ECON1     _u(0x1F)

// Bank 3 Registers
#define MAADR5     _u(0x00)
#define MAADR6     _u(0x01)
#define MAADR3     _u(0x02)
#define MAADR4     _u(0x03)
#define MAADR1     _u(0x04)
#define MAADR2     _u(0x05)
#define EBSTSD     _u(0x06)
#define EBSTCON    _u(0x07)
#define EBSTCSL    _u(0x08)
#define EBSTCSH    _u(0x09)
#define MISTAT     _u(0x0A)
// #define —          _u(0x0B)
// #define —          _u(0x0C)
// #define —          _u(0x0D)
// #define —          _u(0x0E)
// #define —          _u(0x0F)
// #define —          _u(0x10)
// #define —          _u(0x11)
#define EREVID     _u(0x12)
// #define —          _u(0x13)
// #define —          _u(0x14)
#define ECOCON     _u(0x15)
// #define Reserved   _u(0x16)
#define EFLOCON    _u(0x17)
#define EPAUSL     _u(0x18)
#define EPAUSH     _u(0x19)
// #define Reserved   _u(0x1A)
#define EIE        _u(0x1B)
#define EIR        _u(0x1C)
#define ESTAT      _u(0x1D)
#define ECON2      _u(0x1E)
#define ECON1      _u(0x1F)

/* PHY registers */
#define PHCON1  0x00
#define PHSTAT1 0x01
#define PHHID1  0x02
#define PHHID2  0x03
#define PHCON2  0x10
#define PHSTAT2 0x11
#define PHIE    0x12
#define PHIR    0x13
#define PHLCON  0x14

#define WS2812_BANK0    0x0
#define WS2812_BANK1    0x1
#define WS2812_BANK2    0x2
#define WS2812_BANK3    0x3

/* buffer boundaries applied to internal 8K ram
 * entire available packet buffer space is allocated.
 * Give TX buffer space for one full ethernet frame (~1500 bytes)
 * receive buffer gets the rest */
#define WS2812_TXSTART_INIT	    0x1A00
#define WS2812_TXEND_INIT       0x1FFF

/* Put RX buffer at 0 as suggested by the Errata datasheet */
#define WS2812_RXSTART_INIT     0x0000
#define WS2812_RXEND_INIT       0x19FF

/* ENC28J60 EIE Register Bit Definitions */
#define EIE_INTIE	0x80
#define EIE_PKTIE	0x40
#define EIE_DMAIE	0x20
#define EIE_LINKIE	0x10
#define EIE_TXIE	0x08
/* #define EIE_WOLIE	0x04 (reserved) */
#define EIE_TXERIE	0x02
#define EIE_RXERIE	0x01
/* ENC28J60 EIR Register Bit Definitions */
#define EIR_PKTIF	0x40
#define EIR_DMAIF	0x20
#define EIR_LINKIF	0x10
#define EIR_TXIF	0x08
/* #define EIR_WOLIF	0x04 (reserved) */
#define EIR_TXERIF	0x02
#define EIR_RXERIF	0x01
/* ENC28J60 ECON1 Register Bit Definitions */
#define ECON1_TXRST	0x80
#define ECON1_RXRST	0x40
#define ECON1_DMAST	0x20
#define ECON1_CSUMEN	0x10
#define ECON1_TXRTS	0x08
#define ECON1_RXEN	0x04
#define ECON1_BSEL1	0x02
#define ECON1_BSEL0	0x01

/* ENC28J60 MACON1 Register Bit Definitions */
#define MACON1_LOOPBK   0x10
#define MACON1_TXPAUS   0x08
#define MACON1_RXPAUS   0x04
#define MACON1_PASSALL  0x02
#define MACON1_MARXEN   0x01
/* ENC28J60 MACON2 Register Bit Definitions */
#define MACON2_MARST    0x80
#define MACON2_RNDRST   0x40
#define MACON2_MARXRST  0x08
#define MACON2_RFUNRST  0x04
#define MACON2_MATXRST  0x02
#define MACON2_TFUNRST  0x01
/* ENC28J60 MACON3 Register Bit Definitions */
#define MACON3_PADCFG2  0x80
#define MACON3_PADCFG1  0x40
#define MACON3_PADCFG0  0x20
#define MACON3_TXCRCEN  0x10
#define MACON3_PHDRLEN  0x08
#define MACON3_HFRMLEN  0x04
#define MACON3_FRMLNEN  0x02
#define MACON3_FULDPX   0x01
/* ENC28J60 MICMD Register Bit Definitions */
#define MICMD_MIISCAN	0x02
#define MICMD_MIIRD	0x01
/* ENC28J60 MISTAT Register Bit Definitions */
#define MISTAT_NVALID	0x04
#define MISTAT_SCAN	0x02
#define MISTAT_BUSY	0x01
/* ENC28J60 ERXFCON Register Bit Definitions */
#define ERXFCON_UCEN    0x80
#define ERXFCON_ANDOR   0x40
#define ERXFCON_CRCEN   0x20
#define ERXFCON_PMEN    0x10
#define ERXFCON_MPEN    0x08
#define ERXFCON_HTEN    0x04
#define ERXFCON_MCEN    0x02
#define ERXFCON_BCEN    0x01

/* ENC28J60 PHY PHCON1 Register Bit Definitions */
#define PHCON1_PRST	0x8000
#define PHCON1_PLOOPBK	0x4000
#define PHCON1_PPWRSV	0x0800
#define PHCON1_PDPXMD	0x0100
/* ENC28J60 PHY PHSTAT1 Register Bit Definitions */
#define PHSTAT1_PFDPX	0x1000
#define PHSTAT1_PHDPX	0x0800
#define PHSTAT1_LLSTAT	0x0004
#define PHSTAT1_JBSTAT	0x0002
/* ENC28J60 PHY PHSTAT2 Register Bit Definitions */
#define PHSTAT2_TXSTAT	(1 << 13)
#define PHSTAT2_RXSTAT	(1 << 12)
#define PHSTAT2_COLSTAT	(1 << 11)
#define PHSTAT2_LSTAT	(1 << 10)
#define PHSTAT2_DPXSTAT	(1 << 9)
#define PHSTAT2_PLRITY	(1 << 5)
/* ENC28J60 PHY PHCON2 Register Bit Definitions */
#define PHCON2_FRCLINK	0x4000
#define PHCON2_TXDIS	0x2000
#define PHCON2_JABBER	0x0400
#define PHCON2_HDLDIS	0x0100
/* ENC28J60 PHY PHIE Register Bit Definitions */
#define PHIE_PLNKIE	(1 << 4)
#define PHIE_PGEIE	(1 << 1)
/* ENC28J60 PHY PHIR Register Bit Definitions */
#define PHIR_PLNKIF	(1 << 4)
#define PHIR_PGEIF	(1 << 1)

/* maximum ethernet frame length */
#define MAX_FRAMELEN    1518

typedef struct enc28j60_spi_device_t {
    spi_inst_t *bus;
    uint32_t cs;
    uint32_t reset;
} enc28j60_spi_device;

int32_t enc28j20_initialize(enc28j60_spi_device *device);
void enc28j60_hw_disable(enc28j60_spi_device *device);
void enc28j60_hw_enable(enc28j60_spi_device *device);

void enc28j20_initialize_rx_fifo(enc28j60_spi_device *device, uint16_t start, uint16_t end);
void enc28j20_initialize_tx_fifo(enc28j60_spi_device *device, uint16_t start, uint16_t end);
int32_t enc28j60_spi_read(enc28j60_spi_device *device, uint8_t reg, uint8_t *buffer, size_t buffer_length);
int32_t enc28j60_spi_write_control(enc28j60_spi_device *device, uint8_t reg, uint8_t data);
int32_t enc28j60_spi_write_phy(enc28j60_spi_device *device, uint8_t reg, uint16_t data);
int16_t enc28j60_spi_read_phy(enc28j60_spi_device *device, uint8_t reg);
void enc28j60_spi_bitfield_clear(enc28j60_spi_device *device, uint8_t reg, uint8_t data);
void enc28j60_spi_bitfield_set(enc28j60_spi_device *device, uint8_t reg, uint8_t data);
uint8_t enc28j60_get_revision(enc28j60_spi_device *device);
int32_t enc28j60_spi_reset(enc28j60_spi_device *device);
int32_t enc28j60_set_bank(enc28j60_spi_device *device, uint8_t bank);
int32_t enc28j60_set_mac(enc28j60_spi_device *device, uint8_t *address);
void enc28j60_check_link_status(enc28j60_spi_device *device);

#endif // ENC28J60_H
