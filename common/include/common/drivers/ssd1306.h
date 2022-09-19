#ifndef SSD1306_H
#define SSD1306_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/mutex.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

// commands (see datasheet)
#define OLED_SET_CONTRAST _u(0x81)
#define OLED_SET_ENTIRE_ON _u(0xA4)
#define OLED_SET_NORM_INV _u(0xA6)
#define OLED_SET_DISP _u(0xAE)
#define OLED_SET_MEM_ADDR _u(0x20)
#define OLED_SET_COL_ADDR _u(0x21)
#define OLED_SET_PAGE_ADDR _u(0x22)
#define OLED_SET_DISP_START_LINE _u(0x40)
#define OLED_SET_SEG_REMAP _u(0xA0)
#define OLED_SET_MUX_RATIO _u(0xA8)
#define OLED_SET_COM_OUT_DIR _u(0xC0)
#define OLED_SET_DISP_OFFSET _u(0xD3)
#define OLED_SET_COM_PIN_CFG _u(0xDA)
#define OLED_SET_DISP_CLK_DIV _u(0xD5)
#define OLED_SET_PRECHARGE _u(0xD9)
#define OLED_SET_VCOM_DESEL _u(0xDB)
#define OLED_SET_CHARGE_PUMP _u(0x8D)
#define OLED_SET_HORIZ_SCROLL _u(0x26)
#define OLED_SET_SCROLL _u(0x2E)

#define OLED_ADDR _u(0x3D)
#define OLED_HEIGHT _u(64)
#define OLED_WIDTH _u(128)
#define OLED_PAGE_HEIGHT _u(8)
#define OLED_NUM_PAGES OLED_HEIGHT / OLED_PAGE_HEIGHT
#define OLED_BUF_LEN (OLED_NUM_PAGES * OLED_WIDTH)

#define OLED_WRITE_MODE _u(0xFE)
#define OLED_READ_MODE _u(0xFF)

#define SSD1306_ADDRESSING_PAGE         0x2
#define SSD1306_ADDRESSING_VERTICAL     0x1
#define SSD1306_ADDRESSING_HORIZONTAL   0x0

typedef struct ssd1306_device {
    i2c_inst_t *bus;
    uint8_t address;
} SSD1306Dev;

typedef struct ssd1306_spi_device_t {
    spi_inst_t *bus;
    // uint32_t sclk;
    // uint32_t mosi;
    uint32_t cs;
    uint32_t dc;
    uint32_t reset;
} ssd1306_spi_device;

enum ssd1306_write_type {
    COMMAND = 0,
    DATA    = 1
};

void ssd1306_write(SSD1306Dev* dev, uint8_t byte);
void ssd1306_write_buffer(SSD1306Dev *dev, uint8_t *buffer, int32_t buffer_length);
void ssd1306_fill_screen(SSD1306Dev *dev, uint8_t byte);
void ssd1306_initialize_device(SSD1306Dev *dev);
void ssd1306_ignore_ram(SSD1306Dev *dev, bool enable);
void ssd1306_set_contrast(SSD1306Dev *dev, uint8_t contrast);
void ssd1306_set_addressing(SSD1306Dev *dev, uint8_t mode);
void ssd1306_reset_cursor(SSD1306Dev *dev);

/**
 * @brief Performs a write to the SSD1306
 * 
 * @param device Desired device to write to
 * @param type Type of write operation being performed
 * @param buffer Buffer of uint8_t bytes to write
 * @param buffer_length Length of the buffer in bytes
 */
void ssd1306_write(ssd1306_spi_device *device, ssd1306_write_type type, const uint8_t *buffer, uint32_t buffer_length);

/**
 * @brief Initialize the SSD1306 
 * 
 * @param dev Desired device to initialize
 */
void ssd1306_initialize_device(ssd1306_spi_device *dev);

/**
 * @brief Enables/Disables the SSD1306 display
 * 
 * @param dev Desired target device
 * @param enable Desired state of the display enable
 */
void ssd1306_set_display_enable(ssd1306_spi_device *dev, bool enable);

/**
 * @brief Set the contrast for the SSD1306 display
 * 
 * @param dev Desired target device
 * @param contrast Desired contrast value, range from 0 - 255 
 */
void ssd1306_set_contrast(ssd1306_spi_device *dev, uint8_t contrast);

/**
 * @brief Instruct the SSD1306 to ignore RAM and turn entire display on or follow
 * RAM
 * 
 * @param device Desired target device
 * @param enable True - Ignore RAM. False - Follow RAM.
 */
void ssd1306_set_ignore_ram(ssd1306_spi_device *device, bool enable);

/**
 * @brief Enables/Disables display inversion. During normal operation 0 = OFF,
 * 1 = ON. During inverted operation 0 = ON, 1 = OFF.
 * 
 * @param device Desired target device
 * @param invert Desired state of the display inversion
 */
void ssd1306_set_invert_display(ssd1306_spi_device *device, bool invert);

/**
 * @brief Sets the memory addressing mode of the SSD1306. 
 * 00b, Horizontal Addressing Mode
 * 01b, Vertical Addressing Mode
 * 10b, Page Addressing Mode (RESET)
 * 11b, Invalid
 * 
 * @param device Desired target device
 * @param mode Desired addressing mode
 */
void ssd1306_set_addressing(ssd1306_spi_device *device, uint8_t mode);

/**
 * @brief Sets the start/end addresses for the columns/pages of the SSD1306
 * 
 * @param device Desired target device
 * @param start_col Starting column address
 * @param end_col Ending column address
 * @param start_page Starting page address
 * @param end_page Ending page address
 */
void ssd1306_set_cursor(ssd1306_spi_device *device, uint8_t start_col, uint8_t end_col, uint8_t start_page, uint8_t end_page);

/**
 * @brief Resets the cursor
 * 
 * @param device Desired target device
 */
void ssd1306_reset_cursor(ssd1306_spi_device *device);

/**
 * @brief Writes to the GDDRAM of the SSD1306
 * 
 * @param device Desired target device
 * @param buffer Buffer of uint8_t bytes to write to the display
 * @param buffer_length Length of the buffer in bytes
 */
void ssd1306_display(ssd1306_spi_device *device, const uint8_t *buffer, uint32_t buffer_length);

class SSD1306
{
public:
    using Page = uint8_t[OLED_WIDTH];
    using DisplayRam = Page[OLED_PAGE_HEIGHT];

    struct DisplayRamWrite {
        uint8_t address;
        DisplayRam ram;
    };

    struct RenderArea {
        uint8_t start_col;
        uint8_t end_col;
        uint8_t start_page;
        uint8_t end_page;

        int buflen;
    };

    enum AddressingMode {
        HORIZONTAL  = 0,
        VERTICAL
    };

    SSD1306(i2c_inst_t *bus, uint8_t address);

    void initialize();
    void ignore_ram(bool enable);
    void render(uint8_t *buffer, RenderArea *area);
    void fill_screen(uint8_t buffer);
    void fill_display(DisplayRam &ram, uint8_t byte = 0x00);
    void fill_display_random(DisplayRam &ram);
    void reset_cursor();
    void set_contrast(uint8_t contrast);
    void set_addressing_mode(AddressingMode mode);

    static void fill(uint8_t *buf, uint8_t fill);
    static void calc_render_area_buflen(struct RenderArea *area);
    void write_data(const uint8_t *buf, int length);
    void write_buffer(const uint8_t buf[], int buflen);
    void write_buffer(DisplayRamWrite &ram);

private:
    mutex_t *mMutex;
    i2c_inst_t *mBus;
    uint8_t mAddress;

    void write(uint8_t data);
};

#endif // SSD1306_H
