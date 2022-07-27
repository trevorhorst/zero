#include "common/drivers/epaper.h"

DrvEPaper::DrvEPaper(spi_inst_t *spi, uint32_t cs, uint32_t dc, uint32_t busy, uint32_t reset) :
    mSpi(spi),
    mPinChipSelect(cs),
    mPinDataCommand(dc),
    mPinBusy(busy),
    mPinReset(reset)
{
    // initialize();
    // uint8_t data[11];
    // read(cmd_otp_read, data, sizeof(data));
    // for(uint32_t i = 0; i < 11; i++) {
    //     LOG_TRACE("READ 0x%02x\n", data[i]);
    // }
}

void DrvEPaper::reset()
{
    gpio_set_dir(mPinReset, GPIO_OUT);
    LOG_TRACE("Display reset HIGH\n");
    gpio_put(mPinReset, 1);
    sleep_ms(100);
    LOG_TRACE("Display reset LOW\n");
    gpio_put(mPinReset, 0);
    sleep_ms(2);
    LOG_TRACE("Display reset HIGH\n");
    gpio_put(mPinReset, 1);
    sleep_ms(100);

    while(isBusy()) {
        sleep_ms(200);
    }
}

void DrvEPaper::sleep() 
{
    //After this command initiated, the chip will
    // enter Deep Sleep Mode, BUSY pad will keep
    // output high.
    // Remark:
    // To Exit Deep Sleep mode, User required to
    // send HWRESET to the driver
    write(COMMAND, 0x10); //enter deep sleep
    write(DATA, 0x01); 
    sleep_ms(100);
}

void DrvEPaper::initialize()
{
    reset();    

    write(COMMAND, Command::SW_RESET);

    while(isBusy()) {
        sleep_ms(200);
    }

    // Init code
    write(COMMAND, Command::DRIVER_OUTPUT);
    write(DATA, 0xC7);
    write(DATA, 0x00);
    write(DATA, 0x01);

    // Data entry sequence setting
    write(COMMAND, Command::DATA_ENTRY_MODE);
    write(DATA, 0x11);

    write(COMMAND, Command::RAM_X_ADDR);
    write(DATA, 0x00);
    write(DATA, 0x18);

    write(COMMAND, Command::RAM_Y_ADDR);
    write(DATA, 0xC7);
    write(DATA, 0x00);
    write(DATA, 0x00);
    write(DATA, 0x00);

    write(COMMAND, Command::BORDER_WAVEFORM);
    write(DATA, 0x05);

    write(COMMAND, Command::BUILTIN_TEMP);
    write(DATA, 0x80);

    write(COMMAND, Command::RAM_X_COUNTER);
    write(DATA, 0x00);

    write(COMMAND, Command::RAM_Y_COUNTER);
    write(DATA, 0xC7);
    write(DATA, 0x00);

    while(isBusy()) {
        sleep_us(50);
        LOG_TRACE("Display busy\n");
    }
    sleep_ms(200);
}

void DrvEPaper::write(uint8_t type, uint8_t byte)
{
    // The Data/Command pin should be low when writing the command
    if(type == WriteType::COMMAND) {
        gpio_put(mPinDataCommand, 0);
    } else {
        gpio_put(mPinDataCommand, 1);
    }

    gpio_put(mPinChipSelect, 0);
    int32_t written = spi_write_blocking(mSpi, &byte, 1);
    if(written != 1) {
        LOG_WARN("Failed to write 0x%02x\n", byte);
    } else {
        LOG_DEBUG("Wrote (%d) 0x%02X\n", type, byte);
    }

    gpio_put(mPinChipSelect, 1);
}

bool DrvEPaper::isBusy()
{
    gpio_set_dir(mPinBusy, GPIO_IN);
    return gpio_get(mPinBusy);
}