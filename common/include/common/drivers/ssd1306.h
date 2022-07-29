#ifndef SSD1306_H
#define SSD1306_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/mutex.h"
#include "hardware/i2c.h"

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

    SSD1306(i2c_inst_t *bus, uint8_t address);

    void initialize();
    void ignore_ram(bool enable);
    void render(uint8_t *buffer, RenderArea *area);
    void fill_screen(uint8_t buffer);
    void fill_display(DisplayRam &ram);
    void fill_display_random(DisplayRam &ram);
    void reset_cursor();

    static void fill(uint8_t buf[], uint8_t fill);
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
