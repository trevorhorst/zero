#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "lwip/inet.h"
#include "lwip/etharp.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/init.h"
#include "lwip/stats.h"
#include "lwip/dhcp.h"
#include "lwip/timeouts.h"

#include "core/logger.h"
#include "core/console/console.h"
#include "core/drivers/enc28j60.h"
#include "core/drivers/ws2812.h"

#include "project/application.h"

#define PIN_SPI0_CS         1      // CS
#define PIN_SPI0_SCK        2      // CLK
#define PIN_SPI0_MOSI       3      // SI
#define PIN_SPI0_MISO       4      // SO
#define PIN_ETHCTL_RESET    5      // RST
#define PIN_NEOPIXEL        16     // WS2812 Neopixel

#define UNIT_MHZ(x) x * 1000000

#define ETHERNET_MTU 1500

enc28j60_spi_device ethernet_controller;
static const uint8_t mac[6] = {0xAA, 0x6F, 0x77, 0x47, 0x75, 0x8C};

void initialize_pinmux()
{
    // Initialize SPI pins
    gpio_set_function(PIN_SPI0_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI0_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SPI0_MISO, GPIO_FUNC_SPI);

    // Configure pin 1 for chip select on the SPI
    gpio_init(PIN_SPI0_CS);
    gpio_set_dir(PIN_SPI0_CS, GPIO_OUT);
    gpio_put(PIN_SPI0_CS, 1);

    // Configure pin 5 for chip select on the SPI
    gpio_init(PIN_ETHCTL_RESET);
    gpio_set_dir(PIN_ETHCTL_RESET, GPIO_OUT);
    gpio_put(PIN_ETHCTL_RESET, 1);
}

void initialize_neopixel(ws2812_device *device)
{
    device->pin = 16;
    device->length = 1;
    device->pio = pio0;
    device->sm = 0;
    device->bytes[0] = WS2812_DATA_BYTE_NONE;
    device->bytes[1] = WS2812_DATA_BYTE_GREEN;
    device->bytes[2] = WS2812_DATA_BYTE_RED;
    device->bytes[3] = WS2812_DATA_BYTE_BLUE;

    // Initialie the pixel to a blank state
    ws2812_initialize(device);
    ws2812_fill(device, WS2812_RGB(0, 0, 0));
    ws2812_show(device);
}

uint32_t spi_reset(int32_t argc, char **argv)
{
    uint32_t error = 0;
    gpio_set_dir(PIN_ETHCTL_RESET, GPIO_OUT);
    gpio_put(PIN_ETHCTL_RESET, 0);
    sleep_ms(1000);
    gpio_put(PIN_ETHCTL_RESET, 1);
    return error;
}

// uint32_t spi_dump(int32_t argc, char **argv)
// {
//     uint32_t error = 0;
//     if(argc == 3) {
//         // We are performing a read operation
//         long bank = strtol(argv[2], NULL, 16);
//         if(errno == ERANGE) {
//             return 1;
//         }
//
//         uint8_t value = 0;
//         enc28j60_set_bank(&ethernet_controller, bank);
//         for(uint8_t i = 0; i < 32; i++) {
//             enc28j60_spi_read(&ethernet_controller, i, &value, sizeof(value));
//             LOG_INFO("[0x%02X] --> 0x%02X\n", i, value);
//         }
//         // LOG_INFO("READ: 0x%04X\n", value);
//     }
//     return error;
// }

// uint32_t spi_read(int32_t argc, char **argv)
// {
//     uint32_t error = 0;
//     LOG_INFO("Read args: %d\n", argc);
//     if(argc == 3) {
//         // We are performing a read operation
//         long address = strtol(argv[2], NULL, 16);
//         if(errno == ERANGE) {
//             return 1;
//         }
//
//         uint8_t value = 0;
//         enc28j60_read_op(&ethernet_controller, ENC28J60_READ_CTRL_REG, address, &value, sizeof(value));
//         LOG_INFO("READ: 0x%04X\n", value);
//     }
//     return error;
// }

// uint32_t spi_write(int32_t argc, char **argv)
// {
//     uint32_t error = 0;
//     LOG_INFO("Write args: %d\n", argc);
//     if(argc == 5) {
//         long address = strtol(argv[2], NULL, 16);
//         if(errno == ERANGE) {
//             return 1;
//         }
//
//         long data = strtol(argv[4], NULL, 16);
//         if(errno == ERANGE) {
//             return 1;
//         }
//
//         enc28j60_spi_write_control(&ethernet_controller, address, data);
//     }
//     return error;
// }

uint32_t phy_read(int32_t argc, char **argv)
{
    uint32_t error = 0;
    LOG_INFO("Read args: %d\n", argc);
    if(argc == 3) {
        // We are performing a read operation
        long address = strtol(argv[2], NULL, 16);
        if(errno == ERANGE) {
            return 1;
        }

        uint16_t value = enc28j60_phy_read(&ethernet_controller, address);
        LOG_INFO("READ: 0x%04X\n", value);
    }

    return error;
}

static err_t netif_output(struct netif *netif, struct pbuf *p)
{
    LINK_STATS_INC(link.xmit);

    // lock_interrupts();
    // pbuf_copy_partial(p, mac_send_buffer, p->tot_len, 0);
    /* Start MAC transmit here */

    printf("enc28j60: Sending packet of len %d\n", p->len);
    enc28j60_packet_send(&ethernet_controller, p->len, (uint8_t *)p->payload);
    // pbuf_free(p);

    // error sending
    if(enc28j60_read(&ethernet_controller, ESTAT) & ESTAT_TXABRT) {
        // a seven-byte transmit status vector will be
        // written to the location pointed to by ETXND + 1,
        printf("ERR - transmit aborted\n");
    }

    if(enc28j60_read(&ethernet_controller, EIR) & EIR_TXERIF) {
        printf("ERR - transmit interrupt flag set\n");
    }

    // unlock_interrupts();
    return ERR_OK;
}

static void netif_status_callback(struct netif *netif)
{
    LOG_INFO("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
}

static err_t netif_initialize(struct netif *netif)
{
    netif->linkoutput = netif_output;
    netif->output = etharp_output;
    // netif->output_ip6 = ethip6_output;
    netif->mtu = ETHERNET_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
    // MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100000000);
    SMEMCPY(netif->hwaddr, mac, sizeof(netif->hwaddr));
    netif->hwaddr_len = sizeof(netif->hwaddr);
    return ERR_OK;
}

void initialize_ethernet_controller(enc28j60_spi_device *device)
{
    LOG_INFO("Initialize Ethernet Controller\n");

    net_device netdev = {{0xAA, 0x6F, 0x77, 0x47, 0x75, 0x8C}, 0, false};

    // Initialize device settings
    device->net = &netdev;
    device->bus = spi0;
    device->cs = PIN_SPI0_CS;
    device->reset = PIN_ETHCTL_RESET;
    device->debug = true;

    spi_init(spi0, UNIT_MHZ(10));
    spi_set_format(
        spi0,
        8,
        SPI_CPOL_0,
        SPI_CPHA_0,
        SPI_MSB_FIRST
    );

    spi_reset(0, NULL);

    LOG_INFO("ENC28J60 Rev: %02X\n", enc28j60_get_rev(device));

    // uint8_t mac_address[6] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};
    // enc28j60_hw_disable(device);
    // enc28j20_initialize(device);
    // enc28j60_set_mac(device, mac_address);
    // enc28j60_hw_enable(device);
    // enc28j60_check_link_status(device);
}

void initialize_ethernet()
{
    LOG_INFO("Initialize Ethernet\n");
    ip_addr_t addr, mask, static_ip;
    IP4_ADDR(&static_ip, 192, 168, 1, 111);
    IP4_ADDR(&mask, 255, 255, 255, 0);
    IP4_ADDR(&addr, 192, 168, 1, 1);

    struct netif netif;
    lwip_init();
    // IP4_ADDR_ANY if using DHCP client
    netif_add(&netif, &static_ip, &mask, &addr, NULL, netif_initialize, netif_input);
    netif.name[0] = 'e';
    netif.name[1] = '0';
    // netif_create_ip6_linklocal_address(&netif, 1);
    // netif.ip6_autoconfig_enabled = 1;
    netif_set_status_callback(&netif, netif_status_callback);
    netif_set_default(&netif);
    netif_set_up(&netif);

    dhcp_inform(&netif);
    enc28j60_initialize(&ethernet_controller);
}

#define CMD_DUMP        "dump"
#define CMD_SPI_READ    "read"
#define CMD_SPI_WRITE   "write"
#define CMD_SPI_RESET   "reset"
#define CMD_PHY_READ    "phyr"

int32_t application_run()
{
    int32_t success = 0;
    stdio_init_all();

    sleep_ms(1000);

    initialize_pinmux();

    // Initialize neopixel device information
    ws2812_device neopixel;
    initialize_neopixel(&neopixel);

    initialize_ethernet_controller(&ethernet_controller);
    // initialize_ethernet();

    LOG_INFO("ENC28J60 Version: %s\n", ENC28J60_VERSION);
    LOG_INFO("  Common Version: %s\n", CORE_VERSION);

    struct console_command cmd_spi_reset = {&spi_reset};
    // struct console_command cmd_spi_dump = {&spi_dump};
    // struct console_command cmd_spi_read = {&spi_read};
    // struct console_command cmd_spi_write = {&spi_write};
    struct console_command cmd_phy_read = {&phy_read};

    // Initialize and launch the console on second core
    console_initialize();
    console_add_command(CMD_SPI_RESET, &cmd_spi_reset);
    // console_add_command(CMD_SPI_READ, &cmd_spi_read);
    // console_add_command(CMD_DUMP, &cmd_spi_dump);
    // console_add_command(CMD_SPI_WRITE, &cmd_spi_write);
    console_add_command(CMD_PHY_READ, &cmd_phy_read);
    multicore_launch_core1(&console_run);

    // LOG_INFO("Initialize Ethernet\n");
    // ip_addr_t addr, mask, static_ip;
    // IP4_ADDR(&static_ip, 192, 168, 1, 8);
    // IP4_ADDR(&mask, 255, 255, 255, 0);
    // IP4_ADDR(&addr, 192, 168, 1, 1);

    // struct netif netif;
    // lwip_init();
    // // IP4_ADDR_ANY if using DHCP client
    // netif_add(&netif, &static_ip, &mask, &addr, NULL, netif_initialize, netif_input);
    // netif.name[0] = 'e';
    // netif.name[1] = '0';
    // // netif_create_ip6_linklocal_address(&netif, 1);
    // // netif.ip6_autoconfig_enabled = 1;
    // netif_set_status_callback(&netif, netif_status_callback);
    // netif_set_default(&netif);
    // netif_set_up(&netif);

    // dhcp_inform(&netif);

    enc28j60_hw_disable(&ethernet_controller);
    enc28j60_initialize(&ethernet_controller);
    enc28j60_hw_enable(&ethernet_controller);
    enc28j60_check_link_status(&ethernet_controller);

    uint8_t *eth_pkt = malloc(ETHERNET_MTU);
    struct pbuf *p = NULL;

    // netif_set_link_up(&netif);

    // Main thread loop
    bool toggle = true;
    while(true) {
        uint16_t packet_len = enc28j60_packet_receive(&ethernet_controller, ETHERNET_MTU, (uint8_t*)eth_pkt);

        if (packet_len) {
            LOG_INFO("enc: Received packet of length = %d\n", packet_len);
            // p = pbuf_alloc(PBUF_RAW, packet_len, PBUF_POOL);
            // pbuf_take(p, eth_pkt, packet_len);
            // free(eth_pkt);
            // eth_pkt = malloc(ETHERNET_MTU);
        } else {
            // printf("enc: no packet received\n");
        }

        if (packet_len && p != NULL) {
            LINK_STATS_INC(link.recv);

            // if (netif.input(p, &netif) != ERR_OK) {
            //     pbuf_free(p);
            // }
        }

        /* Cyclic lwIP timers check */
        sys_check_timeouts();


        if(toggle) {
            ws2812_fill(&neopixel, WS2812_RGB(0, 25, 0));
            ws2812_show(&neopixel);
        } else {
            ws2812_fill(&neopixel, WS2812_RGB(0, 0, 0));
            ws2812_show(&neopixel);
        }
        toggle = !toggle;
        sleep_ms(1000);
    }

    // For posterity
    ws2812_uninitialize(&neopixel);

    return success;
}
