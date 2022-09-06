#include "common/drivers/ssd1306.h"

void ssd1306_write(SSD1306Dev *dev, uint8_t byte) 
{
    uint8_t buf[2] = {0x80, byte};
    i2c_write_blocking(dev->bus, (dev->address & OLED_WRITE_MODE), buf, 2, false);
}

void ssd1306_write_buffer(SSD1306Dev *dev, uint8_t *buffer, int32_t buffer_length)
{
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one gooooooo!

    // copy our frame buffer into a new buffer because we need to add the control byte
    // to the beginning

    // TODO find a more memory-efficient way to do this..
    // maybe break the data transfer into pages?

    uint8_t control = 0x40;

    // for(int32_t i = 1; i < buffer_length + 1; i++) {
    //     temp_buf[i] = buffer[i - 1];
    // }
    // Co = 0, D/C = 1 => the driver expects data to be written to RAM
    i2c_write_blocking(dev->bus, (dev->address & OLED_WRITE_MODE), &control, 1, false);
    i2c_write_raw_blocking(dev->bus, buffer, buffer_length);

    // free(temp_buf);
}

void ssd1306_fill_screen(SSD1306Dev *dev, uint8_t byte)
{
    ssd1306_write(dev, OLED_SET_COL_ADDR);
    ssd1306_write(dev, 0);
    ssd1306_write(dev, 0);

    ssd1306_write(dev, OLED_SET_PAGE_ADDR);
    ssd1306_write(dev, 0);
    ssd1306_write(dev, 0);
}

void ssd1306_initialize_device(SSD1306Dev *dev)
{
    // some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like

    // some configuration values are recommended by the board manufacturer

    ssd1306_write(dev, OLED_SET_DISP | 0x00); // set display off

    /* timing and driving scheme */
    ssd1306_write(dev, OLED_SET_DISP_CLK_DIV); // set display clock divide ratio
    ssd1306_write(dev, 0x80); // div ratio of 1, standard freq

    ssd1306_write(dev, OLED_SET_MUX_RATIO); // set multiplex ratio
    ssd1306_write(dev, OLED_HEIGHT - 1); // our display is only 32 pixels high

    ssd1306_write(dev, OLED_SET_DISP_OFFSET); // set display offset
    ssd1306_write(dev, 0x00); // no offset

    /* resolution and layout */
    ssd1306_write(dev, OLED_SET_DISP_START_LINE); // set display start line to 0

    ssd1306_write(dev, OLED_SET_CHARGE_PUMP); // set charge pump
    ssd1306_write(dev, 0x14); // Vcc internally generated on our board

    /* memory mapping 
    */
    ssd1306_write(dev, OLED_SET_MEM_ADDR); // set memory address mode
    ssd1306_write(dev, 0x00); // horizontal addressing mode

    ssd1306_write(dev, OLED_SET_SEG_REMAP | 0x01); // set segment re-map
    // column address 127 is mapped to SEG0

    ssd1306_write(dev, OLED_SET_COM_OUT_DIR | 0x08); // set COM (common) output scan direction
    // scan from bottom up, COM[N-1] to COM0

    ssd1306_write(dev, OLED_SET_COM_PIN_CFG); // set COM (common) pins hardware configuration
    ssd1306_write(dev, 0x12); // manufacturer magic number

    /* display */
    ssd1306_write(dev, OLED_SET_CONTRAST); // set contrast control
    ssd1306_write(dev, 0xCF);

    ssd1306_write(dev, OLED_SET_PRECHARGE); // set pre-charge period
    ssd1306_write(dev, 0xF1); // Vcc internally generated on our board

    ssd1306_write(dev, OLED_SET_VCOM_DESEL); // set VCOMH deselect level
    ssd1306_write(dev, 0x40); // 0.83xVcc

    ssd1306_write(dev, OLED_SET_ENTIRE_ON); // set entire display on to follow RAM content

    ssd1306_write(dev, OLED_SET_NORM_INV); // set normal (not inverted) display

    // ssd1306_write(dev, OLED_SET_SCROLL | 0x00); // deactivate horizontal scrolling if set
    // this is necessary as memory writes will corrupt if scrolling was enabled
    ssd1306_write(dev, 0x00);
    ssd1306_write(dev, 0x10);
    ssd1306_write(dev, 0x40);

    ssd1306_write(dev, OLED_SET_DISP | 0x01); // turn display on
}

void ssd1306_ignore_ram(SSD1306Dev *dev, bool enable)
{
    if(enable) {
        // Ignore RAM, all pixels on
        ssd1306_write(dev, 0xA5);
    } else {
        // Follow RAM
        ssd1306_write(dev, 0x7F);
    }
}

void ssd1306_set_contrast(SSD1306Dev *dev, uint8_t contrast)
{
    ssd1306_write(dev, OLED_SET_CONTRAST); // set contrast control
    ssd1306_write(dev, contrast);
}


void SSD1306::fill(uint8_t buf[], uint8_t fill) {
    // fill entire buffer with the same byte
    for (int i = 0; i < OLED_BUF_LEN; i++) {
        buf[i] = fill;
    }
}

void SSD1306::calc_render_area_buflen(struct RenderArea *area) 
{
    // calculate how long the flattened buffer will be for a render area
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

/**
 * @brief Construct a new SSD1306 object
 */
SSD1306::SSD1306(i2c_inst_t *bus, uint8_t address) :
    mMutex(nullptr),
    mBus(bus),
    mAddress(address)
{
    mutex_init(mMutex);
}

void SSD1306::write(uint8_t data)
{
    uint8_t buf[2] = {0x80, data};
    i2c_write_blocking(mBus, (mAddress & OLED_WRITE_MODE), buf, 2, false);
}

void SSD1306::write_buffer(const uint8_t buffer[], int bufferLen)
{
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one gooooooo!

    // copy our frame buffer into a new buffer because we need to add the control byte
    // to the beginning

    // TODO find a more memory-efficient way to do this..
    // maybe break the data transfer into pages?
    // uint8_t *temp_buf = (uint8_t*)malloc(bufferLen + 1);

    // for (int i = 1; i < bufferLen + 1; i++) {
    //     temp_buf[i] = buffer[i - 1];
    // }
    // // Co = 0, D/C = 1 => the driver expects data to be written to RAM
    // temp_buf[0] = 0x40;
    uint8_t control_byte = 0x40;
    i2c_write_blocking(mBus, (mAddress & OLED_WRITE_MODE), &control_byte, 1, false);
    i2c_write_raw_blocking(mBus, buffer, bufferLen);

    // free(temp_buf);
}

void SSD1306::write_buffer(DisplayRamWrite &ram)
{
    uint8_t *temp_buf = (uint8_t*)(&ram);
    // for(uint32_t page = 0; page < OLED_PAGE_HEIGHT; page++) {
    //     for(uint32_t column = 0; OLED_WIDTH; column++) {
    //         uint8_t i = (page * OLED_WIDTH) + column;
    //         temp_buf[i] = ram[page][column];
    //     }
    // }
    temp_buf[0] = 0x40;
    i2c_write_blocking(mBus, (mAddress & OLED_WRITE_MODE), temp_buf, sizeof(DisplayRamWrite), false);

    // free(temp_buf);
}

void SSD1306::initialize()
{
    // some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like

    // some configuration values are recommended by the board manufacturer

    write(OLED_SET_DISP | 0x00); // set display off

    /* timing and driving scheme */
    write(OLED_SET_DISP_CLK_DIV); // set display clock divide ratio
    write(0x80); // div ratio of 1, standard freq

    write(OLED_SET_MUX_RATIO); // set multiplex ratio
    write(OLED_HEIGHT - 1); // our display is only 32 pixels high

    write(OLED_SET_DISP_OFFSET); // set display offset
    write(0x00); // no offset

    /* resolution and layout */
    write(OLED_SET_DISP_START_LINE); // set display start line to 0

    write(OLED_SET_CHARGE_PUMP); // set charge pump
    write(0x14); // Vcc internally generated on our board

    /* memory mapping 
    */
    write(OLED_SET_MEM_ADDR); // set memory address mode
    write(0x00); // horizontal addressing mode

    write(OLED_SET_SEG_REMAP | 0x01); // set segment re-map
    // column address 127 is mapped to SEG0

    write(OLED_SET_COM_OUT_DIR | 0x08); // set COM (common) output scan direction
    // scan from bottom up, COM[N-1] to COM0

    write(OLED_SET_COM_PIN_CFG); // set COM (common) pins hardware configuration
    write(0x12); // manufacturer magic number

    /* display */
    write(OLED_SET_CONTRAST); // set contrast control
    write(0xCF);

    write(OLED_SET_PRECHARGE); // set pre-charge period
    write(0xF1); // Vcc internally generated on our board

    write(OLED_SET_VCOM_DESEL); // set VCOMH deselect level
    write(0x40); // 0.83xVcc

    write(OLED_SET_ENTIRE_ON); // set entire display on to follow RAM content

    write(OLED_SET_NORM_INV); // set normal (not inverted) display

    // write(OLED_SET_SCROLL | 0x00); // deactivate horizontal scrolling if set
    // this is necessary as memory writes will corrupt if scrolling was enabled
    write(0x00);
    write(0x10);
    write(0x40);

    write(OLED_SET_DISP | 0x01); // turn display on
}

void SSD1306::ignore_ram(bool enable)
{
    if(enable) {
        // Ignore RAM, all pixels on
        write(0xA5);
    } else {
        // Follow RAM
        write(0x7F);
    }
}

void SSD1306::render(uint8_t *buffer, RenderArea *area)
{
    // update a portion of the display with a render area
    write(OLED_SET_COL_ADDR);
    write(area->start_col);
    write(area->end_col);

    write(OLED_SET_PAGE_ADDR);
    write(area->start_page);
    write(area->end_page);

    write_buffer(buffer, area->buflen);
}

void SSD1306::fill_screen(uint8_t buffer)
{
    // initialize render area for entire frame (128 pixels by 4 pages)
    struct SSD1306::RenderArea frame_area = {start_col: 0, end_col : OLED_WIDTH - 1, start_page : 0, end_page : OLED_NUM_PAGES -
                                                                                                        1};
    SSD1306::calc_render_area_buflen(&frame_area);
    uint8_t buf[OLED_BUF_LEN];
    fill(buf, buffer);
    render(buf, &frame_area);
}

void SSD1306::fill_display(DisplayRam &ram, uint8_t byte)
{
    for(uint32_t page = 0; page < OLED_PAGE_HEIGHT; page++) {
        for(uint32_t column = 0; column < OLED_WIDTH; column++) {
            ram[page][column] = byte;
        }
    }
}

void SSD1306::fill_display_random(DisplayRam &ram)
{
    for(uint32_t page = 0; page < OLED_PAGE_HEIGHT; page++) {
        for(uint32_t column = 0; column < OLED_WIDTH; column++) {
            ram[page][column] = rand() % 255;
        }
    }
}

void SSD1306::set_contrast(uint8_t contrast)
{
    write(OLED_SET_CONTRAST); // set contrast control
    write(contrast);
}

void SSD1306::set_addressing_mode(AddressingMode mode)
{
    write(OLED_SET_MEM_ADDR);
    switch(mode) {
    case AddressingMode::HORIZONTAL :
        write(0x00); // horizontal addressing mode
        break;
    case AddressingMode::VERTICAL :
        write(0x01); // horizontal addressing mode
        break;
    default:
        break;
    }
}

void SSD1306::reset_cursor()
{
    static const uint8_t config22[] = {0x21, 0x00, 0x7F};             // Display ON
    static const uint8_t config23[] = {0x22, 0x00, 0x07};             // Display ON

    write_buffer(config22, 3);
    write_buffer(config23, 3);
}
