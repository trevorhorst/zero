#include "common/drivers/ssd1306.h"

void ssd1306_write(SSD1306Dev *dev, uint8_t byte) 
{
    uint8_t buf[2] = {0x80, byte};
    i2c_write_blocking(dev->bus, (dev->address & OLED_WRITE_MODE), buf, 2, false);
}

void ssd1306_write_buffer(SSD1306Dev *dev, uint8_t *buffer, int32_t buffer_length)
{
    uint8_t *temp_buf = buffer;
    temp_buf[0] = 0x40;

    // uint8_t *temp = &buffer[1];
    // for(int i = 0; i < buffer_length - 1; i++) {
    //     // temp_buf[i] = buffer[i - 1];
    //     if(i % 8 == 0) {
    //         printf("\n%02d: ", i / 8);
    //     }
    //     for(int8_t bit = 0; bit < 8; bit++) {
    //         uint8_t shift = 1 << (bit);
    //         if(temp[i] & shift) {
    //             printf("0");
    //         } else {
    //             printf(".");
    //         }
    //     }
    //     // printf("%02X ", temp_buf[i]);
    // }
    // printf("\n");

    i2c_write_blocking(dev->bus, (dev->address & OLED_WRITE_MODE), temp_buf, buffer_length, false);
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

void ssd1306_set_addressing(SSD1306Dev *dev, uint8_t mode)
{
    ssd1306_write(dev, OLED_SET_MEM_ADDR);
    ssd1306_write(dev, mode);
}

void ssd1306_reset_cursor(SSD1306Dev *dev)
{
    ssd1306_write(dev, OLED_SET_COL_ADDR);
    ssd1306_write(dev, 0x00);
    ssd1306_write(dev, 0x7F);

    ssd1306_write(dev, OLED_SET_PAGE_ADDR);
    ssd1306_write(dev, 0x00);
    ssd1306_write(dev, 0x07);
}


void ssd1306_write(ssd1306_spi_device *device, ssd1306_write_type type, const uint8_t *buffer,
                   uint32_t buffer_length)
{
    gpio_set_dir(device->cs, GPIO_OUT);
    gpio_set_dir(device->dc, GPIO_OUT);

    gpio_put(device->cs, 0);
    switch(type) {
    case ssd1306_write_type::COMMAND:
        gpio_put(device->dc, 0);
        break;
    case ssd1306_write_type::DATA:
        gpio_put(device->dc, 1);
        break;
    default:
        break;
    }

    spi_write_blocking(device->bus, buffer, buffer_length);
    gpio_put(device->cs, 1);
}

void ssd1306_display(ssd1306_spi_device *device, const uint8_t *buffer, uint32_t buffer_length)
{
    gpio_set_dir(device->cs, GPIO_OUT);
    gpio_set_dir(device->dc, GPIO_OUT);

    gpio_put(device->cs, 0);
    gpio_put(device->dc, 1);

    spi_write_blocking(device->bus, buffer, buffer_length);
    gpio_put(device->cs, 1);
}

void ssd1306_reset_device(ssd1306_spi_device *device)
{
    gpio_set_dir(device->reset, GPIO_OUT);
    gpio_put(device->reset, 1);
    sleep_ms(1);
    gpio_put(device->reset, 0);
    sleep_ms(10);
    gpio_put(device->reset, 1);
}

void ssd1306_initialize_device(ssd1306_spi_device *dev)
{
    // some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like

    // some configuration values are recommended by the board manufacturer

    ssd1306_reset_device(dev);

    ssd1306_set_display_enable(dev, false);

    /* timing and driving scheme */
    uint8_t init2[] = {OLED_SET_DISP_CLK_DIV, 0x80}; // Set display clock divide ratio, div ratio of 1, standard freq
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init2, sizeof(init2)); // div ratio of 1, standard freq

    uint8_t init3[] = {OLED_SET_MUX_RATIO, (OLED_HEIGHT - 1)}; // Set multiplex ratio, display is only 32 pixels high
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init3, sizeof(init3)); // our display is only 32 pixels high

    // Set display offset, no offset
    uint8_t init4[] = {OLED_SET_DISP_OFFSET, 0x00};
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init4, sizeof(init4));

    /* resolution and layout */
    // Set display start line to 0
    uint8_t init5[] = {OLED_SET_DISP_START_LINE};
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init5, sizeof(init5)); // set display start line to 0

    // Set charge pump, vcc internally generated
    uint8_t init6[] = {OLED_SET_CHARGE_PUMP, 0x14};
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init6, sizeof(init6));

    /* memory mapping */
    // Set memory address mode, horizontal addressing
    uint8_t init7[] = {OLED_SET_MEM_ADDR, 0x00};
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init7, sizeof(init7));

    uint8_t init8[] = {(OLED_SET_SEG_REMAP | 0x1)};
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init8, sizeof(init8)); // set segment re-map
    // column address 127 is mapped to SEG0

    uint8_t init9[] = {(OLED_SET_COM_OUT_DIR | 0x8)};
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init9, sizeof(init9)); // set COM (common) output scan direction
    // scan from bottom up, COM[N-1] to COM0

    uint8_t init10[] = {OLED_SET_COM_PIN_CFG, 0x12}; // Set COM (common) pins hardware config, manufacturer magic number
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init10, sizeof(init10));
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, OLED_SET_COM_PIN_CFG, 1); // set COM (common) pins hardware configuration
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, 0x12, 1); // manufacturer magic number

    /* display */
    // Set contrast control
    ssd1306_set_contrast(dev, 0x01);

    uint8_t init12[] = {OLED_SET_PRECHARGE, 0xF1};
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init12, sizeof(init12));
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, OLED_SET_PRECHARGE, 1); // set pre-charge period
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, 0xF1, 1); // Vcc internally generated on our board

    uint8_t init13[] = {OLED_SET_VCOM_DESEL, 0x40};
    ssd1306_write(dev, ssd1306_write_type::COMMAND, init13, sizeof(init13));
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, OLED_SET_VCOM_DESEL, 1); // set VCOMH deselect level
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, 0x40, 1); // 0.83xVcc

    // set entire display on to follow RAM content
    ssd1306_set_ignore_ram(dev, false);
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, OLED_SET_ENTIRE_ON, 1); // set entire display on to follow RAM content

    // Set normal (not inverted) display
    ssd1306_set_invert_display(dev, false);
    // uint8_t init15[] = {OLED_SET_NORM_INV};
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, init15, sizeof(init15));
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, OLED_SET_NORM_INV, 1); // set normal (not inverted) display

    // ssd1306_write(dev, ssd1306_write_type::COMMAND, OLED_SET_SCROLL | 0x00, 1); // deactivate horizontal scrolling if set
    // this is necessary as memory writes will corrupt if scrolling was enabled
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, 0x00, 1);
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, 0x10, 1);
    // ssd1306_write(dev, ssd1306_write_type::COMMAND, 0x40, 1);

    ssd1306_set_display_enable(dev, true);
}

void ssd1306_set_display_enable(ssd1306_spi_device *device, bool enable)
{
    uint8_t command = OLED_SET_DISP;
    if(enable) {
        // Enable display 0xAF
        command |= 0x01;
    }
    ssd1306_write(device, ssd1306_write_type::COMMAND, &command, 1);
}

void ssd1306_set_contrast(ssd1306_spi_device *device, uint8_t contrast)
{
    // Contrast is a 2 byte command
    uint8_t command[2] = {OLED_SET_CONTRAST, contrast};
    ssd1306_write(device, ssd1306_write_type::COMMAND, command, sizeof(command));
}

void ssd1306_set_ignore_ram(ssd1306_spi_device *device, bool enable)
{
    uint8_t command = OLED_SET_ENTIRE_ON;
    if(enable) {
        // Ignore RAM, all pixels on 0xA5
        command |= 0x01;
    }
    ssd1306_write(device, ssd1306_write_type::COMMAND, &command, 1);
}

void ssd1306_set_invert_display(ssd1306_spi_device *device, bool invert)
{
    uint8_t command = OLED_SET_NORM_INV;
    if(invert) {
        // Invert the display (0 = ON, 1 = OFF), all pixels on 0xA5
        command |= 0x01;
    }
    ssd1306_write(device, ssd1306_write_type::COMMAND, &command, 1);
}

void ssd1306_set_cursor(ssd1306_spi_device *device, uint8_t start_col, uint8_t end_col, uint8_t start_page, uint8_t end_page)
{
    // Indicates start column and end column
    uint8_t init_col[] = {OLED_SET_COL_ADDR, start_col, end_col};
    ssd1306_write(device, ssd1306_write_type::COMMAND, init_col, sizeof(init_col));

    // Indicates start page and end page
    uint8_t init_page[] = {OLED_SET_PAGE_ADDR, start_page, end_page};
    ssd1306_write(device, ssd1306_write_type::COMMAND, init_page, sizeof(init_page));
}

void ssd1306_reset_cursor(ssd1306_spi_device *device)
{
    ssd1306_set_cursor(device, 0, (OLED_WIDTH - 1), 0, (OLED_NUM_PAGES - 1));
    // // Indicates start column and end column
    // uint8_t init_col[] = {OLED_SET_COL_ADDR, 0x00, 0x7F};
    // ssd1306_write(device, ssd1306_write_type::COMMAND, init_col, sizeof(init_col));

    // // Indicates start page and end page
    // uint8_t init_page[] = {OLED_SET_PAGE_ADDR, 0x00, 0x07};
    // ssd1306_write(device, ssd1306_write_type::COMMAND, init_page, sizeof(init_page));
}

void ssd1306_fill_screen(ssd1306_spi_device *device, uint8_t byte)
{
    uint8_t buffer[OLED_NUM_PAGES][128];
}

void ssd1306_set_addressing(ssd1306_spi_device *device, uint8_t mode)
{
    uint8_t init_addressing[2] = {OLED_SET_MEM_ADDR, mode};
    ssd1306_write(device, ssd1306_write_type::COMMAND, init_addressing, sizeof(init_addressing));
}



























void SSD1306::fill(uint8_t *buf, uint8_t fill) {
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
