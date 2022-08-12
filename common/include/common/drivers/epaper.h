#ifndef RP2040_DRIVERS_EPAPER_H
#define RP2040_DRIVERS_EPAPER_H

#include <hardware/gpio.h>
#include <hardware/spi.h>

#include "common/logger.h"

#define EPD_1IN54_V2_WIDTH       200
#define EPD_1IN54_V2_HEIGHT      200

class DrvEPaper
{
public:
    enum WriteType : uint8_t {
        COMMAND = 0,
        DATA    = 1
    };

    enum Command : uint8_t {
        DRIVER_OUTPUT           = 0x01,
        GATE_DRIVING_VOLTAGE    = 0x03,
        SOURCE_DRIVING_VOLTAGE  = 0x04,
        DEEP_SLEEP_MODE         = 0x10,
        DATA_ENTRY_MODE         = 0x11,
        SW_RESET                = 0x12,
        BUILTIN_TEMP            = 0x18,
        TEMP_LUT                = 0x1A,
        MASTER_ACTIVATION       = 0x20,
        DISPLAY_UPDATE          = 0x22,
        WRITE_RAM_BW            = 0x24,
        WRITE_RAM_RED           = 0x26,
        OTP_READ                = 0x2D,
        BORDER_WAVEFORM         = 0x3C,
        RAM_X_ADDR              = 0x44,
        RAM_Y_ADDR              = 0x45,
        RAM_X_COUNTER           = 0x4E,
        RAM_Y_COUNTER           = 0x4F,
    };

    DrvEPaper(spi_inst_t *spi, uint32_t cs, uint32_t dc, uint32_t busy, uint32_t reset);

    void initialize();
    void write(uint8_t type, uint8_t byte);
    // void write(uint8_t cmd, const uint8_t *data, const uint32_t bytes);
    // void read(uint8_t cmd, uint8_t *data, uint32_t bytes);
    void reset();
    void wake();
    void sleep();
    void display(uint8_t *image);
    void fillScreen(uint8_t byte);

    bool isBusy();

private:
    spi_inst_t *mSpi;

    const uint32_t mPinChipSelect;
    const uint32_t mPinDataCommand;
    const uint32_t mPinBusy;
    const uint32_t mPinReset;
};

#endif // RP2040_DRIVERS_EPAPER_H