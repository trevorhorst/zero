#include <string.h>

#include "core/console/console.h"
#include "core/drivers/at24c.h"
#include "core/logger.h"
#include "core/tools/i2c.h"

#include "project/application.h"

#include "hardware/i2c.h"

// I2C Pins
#define PIN_I2C1_SDA    4
#define PIN_I2C1_SCL    5

#define BUS_SPEED_KHZ(x) x * 1000

#define CMD_I2C     "scan"
#define CMD_EEPROM  "eeprom"
#define CMD_R32     "r32"
#define CMD_W32     "w32"
#define CMD_D32     "d32"

at24cxxx_i2c_device eeprom;

void initialize_i2c(i2c_inst_t *bus)
{
    LOG_INFO("Initializing I2C...\n");
    i2c_init(bus, BUS_SPEED_KHZ(100));
    gpio_set_function(PIN_I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_I2C1_SDA);
    gpio_pull_up(PIN_I2C1_SCL);

    // tools_i2c_bus_scan(bus);
}

void initialize_eeprom(at24cxxx_i2c_device *device)
{
    LOG_INFO("Initializing EEPROM...\n");
    device->address = AT24CXXX_I2C_ADDRESS(0, 1);
    device->bus     = i2c0;
    LOG_INFO("  EEPROM @ 0x%02X\n", device->address);
}

uint32_t i2c_scan(int32_t argc, char **argv)
{
    uint32_t error = 0;
    tools_i2c_bus_scan(i2c0);
    return error;
}

uint32_t eeprom_operation(int32_t argc, char **argv)
{
    uint32_t error = 0;
    LOG_INFO("EEPROM Operation Argc %d\n", argc);
    if(argc == 3) {
        if(strncmp(CMD_R32, argv[1], 128) == 0) {
            // We are performing a read operation
            long address = strtol(argv[2], NULL, 16);
            if(errno == ERANGE) {
                return 1;
            }

            uint8_t buffer = 0x00;
            at24cxxx_random_read(&eeprom, address, &buffer, sizeof(buffer));
            LOG_INFO("0x%02X --> 0x%02X\n", address, buffer);
        }
    } else if(argc == 5) {
        if(strncmp(CMD_W32, argv[1], 128) == 0) {
            // We are performing a read operation
            long address = strtol(argv[2], NULL, 16);
            if(errno == ERANGE) {
                return 1;
            }

            long data = strtol(argv[4], NULL, 16);
            if(errno == ERANGE) {
                return 1;
            }

            uint8_t buffer[3] = {0, (uint8_t)address, (uint8_t)data};
            at24cxxx_byte_write(&eeprom, buffer, sizeof(buffer));
            LOG_INFO("0x%02X <-- 0x%02X\n", address, data);
        }

    }
    return error;
}

int32_t application_run()
{
    int32_t success = 0;

    stdio_init_all();

    sleep_ms(1000);

    initialize_i2c(i2c0);
    initialize_eeprom(&eeprom);

    struct console_command cmd_i2c = {&i2c_scan};
    struct console_command cmd_eeprom = {&eeprom_operation};

    console_initialize();
    console_add_command(CMD_I2C, &cmd_i2c);
    console_add_command(CMD_EEPROM, &cmd_eeprom);
    multicore_launch_core1(&console_run);

    LOG_INFO("Devices Version: %s\n", DEVICES_VERSION);
    LOG_INFO(" Common Version: %s\n", CORE_VERSION);

    while(true) {

    }

    return success;
}