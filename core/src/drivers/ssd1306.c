#include "core/drivers/ssd1306.h"

int32_t ssd1306_i2c_write(ssd1306_i2c_device *device, const uint8_t *buffer, uint32_t buffer_length)
{
    return i2c_write_blocking(device->bus, device->address, buffer, buffer_length, false);
}

int32_t ssd1306_i2c_write_data(ssd1306_i2c_device *device, const uint8_t *buffer, uint32_t buffer_length)
{
    return ssd1306_i2c_write_raw(device, SSD1306_WRITE_DATA, buffer, buffer_length);
}

int32_t ssd1306_i2c_write_command(ssd1306_i2c_device *device, const uint8_t *buffer, uint32_t buffer_length)
{
    return ssd1306_i2c_write_raw(device, SSD1306_WRITE_COMMAND, buffer, buffer_length);
}

int32_t ssd1306_i2c_write_raw(ssd1306_i2c_device *device, enum ssd1306_write_type type,
                              const uint8_t *buffer, uint32_t buffer_length)
{
    i2c_inst_t *i2c = device->bus;
    uint8_t addr = device->address;
    size_t len = buffer_length;
    const uint8_t *src = buffer;
    bool nostop = false;

    invalid_params_if(I2C, addr >= 0x80); // 7-bit addresses
    // invalid_params_if(I2C, i2c_reserved_addr(addr));
    // Synopsys hw accepts start/stop flags alongside data items in the same
    // FIFO word, so no 0 byte transfers.
    invalid_params_if(I2C, len == 0);
    invalid_params_if(I2C, ((int)len) < 0);

    i2c->hw->enable = 0;
    i2c->hw->tar = addr;
    i2c->hw->enable = 1;

    bool abort = false;
    bool timeout = false;

    uint8_t control_byte = 0;
    if(type == SSD1306_WRITE_DATA) {
        control_byte = OLED_I2C_CONTROL_BYTE(0, 1);
    } else if(type == SSD1306_WRITE_COMMAND) {
        control_byte = OLED_I2C_CONTROL_BYTE(0, 0);
    }

    i2c->hw->data_cmd =
            bool_to_bit(1 && i2c->restart_on_next) << I2C_IC_DATA_CMD_RESTART_LSB |
            bool_to_bit(0 && !nostop) << I2C_IC_DATA_CMD_STOP_LSB |
            control_byte;

    // Wait until the transmission of the address/data from the internal
    // shift register has completed. For this to function correctly, the
    // TX_EMPTY_CTRL flag in IC_CON must be set. The TX_EMPTY_CTRL flag
    // was set in i2c_init.
    do {
        // if (timeout_check) {
        //     timeout = timeout_check(ts);
        //     abort |= timeout;
        // }
        tight_loop_contents();
    } while (!timeout && !(i2c->hw->raw_intr_stat & I2C_IC_RAW_INTR_STAT_TX_EMPTY_BITS));

    uint32_t abort_reason = 0;
    int byte_ctr;

    int ilen = (int)len;
    for (byte_ctr = 0; byte_ctr < ilen; ++byte_ctr) {
        // bool first = byte_ctr == 0;
        bool first = 0;
        bool last = byte_ctr == ilen - 1;

        i2c->hw->data_cmd =
                bool_to_bit(first && i2c->restart_on_next) << I2C_IC_DATA_CMD_RESTART_LSB |
                bool_to_bit(last && !nostop) << I2C_IC_DATA_CMD_STOP_LSB |
                *src++;

        // Wait until the transmission of the address/data from the internal
        // shift register has completed. For this to function correctly, the
        // TX_EMPTY_CTRL flag in IC_CON must be set. The TX_EMPTY_CTRL flag
        // was set in i2c_init.
        do {
            // if (timeout_check) {
            //     timeout = timeout_check(ts);
            //     abort |= timeout;
            // }
            tight_loop_contents();
        } while (!timeout && !(i2c->hw->raw_intr_stat & I2C_IC_RAW_INTR_STAT_TX_EMPTY_BITS));

        // If there was a timeout, don't attempt to do anything else.
        if (!timeout) {
            abort_reason = i2c->hw->tx_abrt_source;
            if (abort_reason) {
                // Note clearing the abort flag also clears the reason, and
                // this instance of flag is clear-on-read! Note also the
                // IC_CLR_TX_ABRT register always reads as 0.
                i2c->hw->clr_tx_abrt;
                abort = true;
            }

            if (abort || (last && !nostop)) {
                // If the transaction was aborted or if it completed
                // successfully wait until the STOP condition has occured.

                // TODO Could there be an abort while waiting for the STOP
                // condition here? If so, additional code would be needed here
                // to take care of the abort.
                do {
                    // if (timeout_check) {
                    //     timeout = timeout_check(ts);
                    //     abort |= timeout;
                    // }
                    tight_loop_contents();
                } while (!timeout && !(i2c->hw->raw_intr_stat & I2C_IC_RAW_INTR_STAT_STOP_DET_BITS));

                // If there was a timeout, don't attempt to do anything else.
                if (!timeout) {
                    i2c->hw->clr_stop_det;
                }
            }
        }

        // Note the hardware issues a STOP automatically on an abort condition.
        // Note also the hardware clears RX FIFO as well as TX on abort,
        // because we set hwparam IC_AVOID_RX_FIFO_FLUSH_ON_TX_ABRT to 0.
        if (abort)
            break;
    }

    int rval;

    // A lot of things could have just happened due to the ingenious and
    // creative design of I2C. Try to figure things out.
    if (abort) {
        if (timeout)
            rval = PICO_ERROR_TIMEOUT;
        else if (!abort_reason || abort_reason & I2C_IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_BITS) {
            // No reported errors - seems to happen if there is nothing connected to the bus.
            // Address byte not acknowledged
            rval = PICO_ERROR_GENERIC;
        } else if (abort_reason & I2C_IC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_BITS) {
            // Address acknowledged, some data not acknowledged
            rval = byte_ctr;
        } else {
            //panic("Unknown abort from I2C instance @%08x: %08x\n", (uint32_t) i2c->hw, abort_reason);
            rval = PICO_ERROR_GENERIC;
        }
    } else {
        rval = byte_ctr;
    }

    // nostop means we are now at the end of a *message* but not the end of a *transfer*
    i2c->restart_on_next = nostop;
    return rval;
}

int32_t ssd1306_i2c_read(ssd1306_i2c_device *device, uint8_t *buffer, uint32_t buffer_length)
{
    return i2c_read_blocking(device->bus, device->address, buffer, buffer_length, false);
}

// int32_t ssd1306_i2c_write(ssd1306_i2c_device *device, enum ssd1306_write_type type, const uint8_t *buffer, uint32_t buffer_length)
// {
//     switch(type) {
//     case SSD1306_WRITE_COMMAND:
//         break;
//     case SSD1306_WRITE_DATA:
//         break;
//     default:
//         break;
//     }
//
//
// }

int32_t ssd1306_i2c_initialize_device(ssd1306_i2c_device *device)
{
    ssd1306_i2c_set_display_enable(device, false);

    /* timing and driving scheme */
    uint8_t init2[] = {OLED_I2C_CONTROL_BYTE(0, 0), OLED_SET_DISP_CLK_DIV, 0x80}; // Set display clock divide ratio, div ratio of 1, standard freq
    ssd1306_i2c_write(device, init2, sizeof(init2)); // div ratio of 1, standard freq

    uint8_t init3[] = {OLED_I2C_CONTROL_BYTE(0, 0), OLED_SET_MUX_RATIO, (OLED_HEIGHT - 1)}; // Set multiplex ratio, display is only 32 pixels high
    ssd1306_i2c_write(device, init3, sizeof(init3)); // our display is only 32 pixels high

    // Set display offset, no offset
    uint8_t init4[] = {OLED_I2C_CONTROL_BYTE(0, 0), OLED_SET_DISP_OFFSET, 0x00};
    ssd1306_i2c_write(device, init4, sizeof(init4));

    /* resolution and layout */
    // Set display start line to 0
    uint8_t init5[] = {OLED_I2C_CONTROL_BYTE(0, 0), OLED_SET_DISP_START_LINE};
    ssd1306_i2c_write(device, init5, sizeof(init5)); // set display start line to 0

    // Set charge pump, vcc internally generated
    uint8_t init6[] = {OLED_I2C_CONTROL_BYTE(0, 0), OLED_SET_CHARGE_PUMP, 0x14};
    ssd1306_i2c_write(device, init6, sizeof(init6));

    /* memory mapping */
    // Set memory address mode, horizontal addressing
    uint8_t init7[] = {OLED_I2C_CONTROL_BYTE(0, 0), OLED_SET_MEM_ADDR, 0x00};
    ssd1306_i2c_write(device, init7, sizeof(init7));

    uint8_t init8[] = {OLED_I2C_CONTROL_BYTE(0, 0), (OLED_SET_SEG_REMAP | 0x1)};
    ssd1306_i2c_write(device, init8, sizeof(init8)); // set segment re-map
    // column address 127 is mapped to SEG0

    uint8_t init9[] = {OLED_I2C_CONTROL_BYTE(0, 0), (OLED_SET_COM_OUT_DIR | 0x8)};
    ssd1306_i2c_write(device, init9, sizeof(init9)); // set COM (common) output scan direction

    uint8_t init10[] = {OLED_I2C_CONTROL_BYTE(0, 0), OLED_SET_COM_PIN_CFG, 0x12}; // Set COM (common) pins hardware config, manufacturer magic number
    ssd1306_i2c_write(device, init10, sizeof(init10));

    /* display */
    // Set contrast control
    ssd1306_i2c_set_contrast(device, 0x01);

    uint8_t init12[] = {OLED_I2C_CONTROL_BYTE(0, 0), OLED_SET_PRECHARGE, 0xF1};
    ssd1306_i2c_write(device, init12, sizeof(init12));

    uint8_t init13[] = {OLED_I2C_CONTROL_BYTE(0, 0), OLED_SET_VCOM_DESEL, 0x40};
    ssd1306_i2c_write(device, init13, sizeof(init13));

    // set entire display on to follow RAM content
    ssd1306_i2c_set_ignore_ram(device, false);

    // Set normal (not inverted) display
    ssd1306_i2c_set_invert_display(device, false);

    ssd1306_i2c_clear_display(device);

    ssd1306_i2c_set_display_enable(device, true);

    return 0;
}

void ssd1306_i2c_set_display_enable(ssd1306_i2c_device *device, bool enable)
{
    uint8_t command[1] = {OLED_SET_DISP};
    if(enable) {
        // Enable display 0xAF
        command[0] |= 0x01;
    }
    ssd1306_i2c_write_command(device, command, sizeof(command));
}

void ssd1306_i2c_set_ignore_ram(ssd1306_i2c_device *device, bool enable)
{
    uint8_t command[1] = {OLED_SET_ENTIRE_ON};
    if(enable) {
        // Ignore RAM, all pixels on 0xA5
        command[0] |= 0x01;
    }
    ssd1306_i2c_write_command(device, command, sizeof(command));
}

void ssd1306_i2c_set_invert_display(ssd1306_i2c_device *device, bool invert)
{
    uint8_t command[1] = {OLED_SET_NORM_INV};
    if(invert) {
        // Invert the display (0 = ON, 1 = OFF), all pixels on 0xA5
        command[0] |= 0x01;
    }
    ssd1306_i2c_write_command(device, command, sizeof(command));
}

void ssd1306_i2c_set_contrast(ssd1306_i2c_device *device, uint8_t contrast)
{
    // Contrast is a 2 byte command
    uint8_t command[2] = {OLED_SET_CONTRAST, contrast};
    ssd1306_i2c_write_command(device, command, sizeof(command));
}

void ssd1306_i2c_set_addressing(ssd1306_i2c_device *device, uint8_t mode)
{
    uint8_t command[2] = {OLED_SET_MEM_ADDR, mode};
    ssd1306_i2c_write_command(device, command, sizeof(command));
}

void ssd1306_i2c_set_cursor(ssd1306_i2c_device *device, uint8_t start_col, uint8_t end_col, uint8_t start_page, uint8_t end_page)
{
    // Indicates start column and end column
    uint8_t init_col[] = {OLED_SET_COL_ADDR, start_col, end_col};
    ssd1306_i2c_write_command(device, init_col, sizeof(init_col));

    // Indicates start page and end page
    uint8_t init_page[] = {OLED_SET_PAGE_ADDR, start_page, end_page};
    ssd1306_i2c_write_command(device, init_page, sizeof(init_page));
}


void ssd1306_i2c_reset_cursor(ssd1306_i2c_device *device)
{
    ssd1306_i2c_set_cursor(device, 0, (OLED_WIDTH - 1), 0, (OLED_NUM_PAGES - 1));
}

void ssd1306_i2c_clear_display(ssd1306_i2c_device *device)
{
    // Write the entire display RAM to 0
    uint8_t buffer[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for(uint32_t i = 0; i < OLED_PAGE_HEIGHT; i++) {
        for(uint32_t j = 0; j < 16; j++) {
            ssd1306_i2c_write(device, buffer, sizeof(buffer));
        }
    }

    // Reset the cursor
    ssd1306_i2c_reset_cursor(device);
}













void ssd1306_write(ssd1306_spi_device *device, enum ssd1306_write_type type, const uint8_t *buffer,
                   uint32_t buffer_length)
{
    gpio_set_dir(device->cs, GPIO_OUT);
    gpio_set_dir(device->dc, GPIO_OUT);

    gpio_put(device->cs, 0);
    switch(type) {
    case SSD1306_WRITE_COMMAND:
        gpio_put(device->dc, 0);
        break;
    case SSD1306_WRITE_DATA:
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
    ssd1306_write(device, SSD1306_WRITE_DATA, buffer, buffer_length);
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
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init2, sizeof(init2)); // div ratio of 1, standard freq

    uint8_t init3[] = {OLED_SET_MUX_RATIO, (OLED_HEIGHT - 1)}; // Set multiplex ratio, display is only 32 pixels high
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init3, sizeof(init3)); // our display is only 32 pixels high

    // Set display offset, no offset
    uint8_t init4[] = {OLED_SET_DISP_OFFSET, 0x00};
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init4, sizeof(init4));

    /* resolution and layout */
    // Set display start line to 0
    uint8_t init5[] = {OLED_SET_DISP_START_LINE};
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init5, sizeof(init5)); // set display start line to 0

    // Set charge pump, vcc internally generated
    uint8_t init6[] = {OLED_SET_CHARGE_PUMP, 0x14};
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init6, sizeof(init6));

    /* memory mapping */
    // Set memory address mode, horizontal addressing
    uint8_t init7[] = {OLED_SET_MEM_ADDR, 0x00};
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init7, sizeof(init7));

    uint8_t init8[] = {(OLED_SET_SEG_REMAP | 0x1)};
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init8, sizeof(init8)); // set segment re-map
    // column address 127 is mapped to SEG0

    uint8_t init9[] = {(OLED_SET_COM_OUT_DIR | 0x8)};
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init9, sizeof(init9)); // set COM (common) output scan direction
    // scan from bottom up, COM[N-1] to COM0

    uint8_t init10[] = {OLED_SET_COM_PIN_CFG, 0x12}; // Set COM (common) pins hardware config, manufacturer magic number
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init10, sizeof(init10));
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, OLED_SET_COM_PIN_CFG, 1); // set COM (common) pins hardware configuration
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, 0x12, 1); // manufacturer magic number

    /* display */
    // Set contrast control
    ssd1306_set_contrast(dev, 0x01);

    uint8_t init12[] = {OLED_SET_PRECHARGE, 0xF1};
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init12, sizeof(init12));
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, OLED_SET_PRECHARGE, 1); // set pre-charge period
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, 0xF1, 1); // Vcc internally generated on our board

    uint8_t init13[] = {OLED_SET_VCOM_DESEL, 0x40};
    ssd1306_write(dev, SSD1306_WRITE_COMMAND, init13, sizeof(init13));
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, OLED_SET_VCOM_DESEL, 1); // set VCOMH deselect level
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, 0x40, 1); // 0.83xVcc

    // set entire display on to follow RAM content
    ssd1306_set_ignore_ram(dev, false);
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, OLED_SET_ENTIRE_ON, 1); // set entire display on to follow RAM content

    // Set normal (not inverted) display
    ssd1306_set_invert_display(dev, false);
    // uint8_t init15[] = {OLED_SET_NORM_INV};
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, init15, sizeof(init15));
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, OLED_SET_NORM_INV, 1); // set normal (not inverted) display

    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, OLED_SET_SCROLL | 0x00, 1); // deactivate horizontal scrolling if set
    // this is necessary as memory writes will corrupt if scrolling was enabled
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, 0x00, 1);
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, 0x10, 1);
    // ssd1306_write(dev, SSD1306_WRITE_COMMAND, 0x40, 1);

    ssd1306_set_display_enable(dev, true);
}

void ssd1306_set_display_enable(ssd1306_spi_device *device, bool enable)
{
    uint8_t command = OLED_SET_DISP;
    if(enable) {
        // Enable display 0xAF
        command |= 0x01;
    }
    ssd1306_write(device, SSD1306_WRITE_COMMAND, &command, 1);
}

void ssd1306_set_contrast(ssd1306_spi_device *device, uint8_t contrast)
{
    // Contrast is a 2 byte command
    uint8_t command[2] = {OLED_SET_CONTRAST, contrast};
    ssd1306_write(device, SSD1306_WRITE_COMMAND, command, sizeof(command));
}

void ssd1306_set_ignore_ram(ssd1306_spi_device *device, bool enable)
{
    uint8_t command = OLED_SET_ENTIRE_ON;
    if(enable) {
        // Ignore RAM, all pixels on 0xA5
        command |= 0x01;
    }
    ssd1306_write(device, SSD1306_WRITE_COMMAND, &command, 1);
}

void ssd1306_set_invert_display(ssd1306_spi_device *device, bool invert)
{
    uint8_t command = OLED_SET_NORM_INV;
    if(invert) {
        // Invert the display (0 = ON, 1 = OFF), all pixels on 0xA5
        command |= 0x01;
    }
    ssd1306_write(device, SSD1306_WRITE_COMMAND, &command, 1);
}

void ssd1306_set_cursor(ssd1306_spi_device *device, uint8_t start_col, uint8_t end_col, uint8_t start_page, uint8_t end_page)
{
    // Indicates start column and end column
    uint8_t init_col[] = {OLED_SET_COL_ADDR, start_col, end_col};
    ssd1306_write(device, SSD1306_WRITE_COMMAND, init_col, sizeof(init_col));

    // Indicates start page and end page
    uint8_t init_page[] = {OLED_SET_PAGE_ADDR, start_page, end_page};
    ssd1306_write(device, SSD1306_WRITE_COMMAND, init_page, sizeof(init_page));
}

void ssd1306_reset_cursor(ssd1306_spi_device *device)
{
    ssd1306_set_cursor(device, 0, (OLED_WIDTH - 1), 0, (OLED_NUM_PAGES - 1));
    // // Indicates start column and end column
    // uint8_t init_col[] = {OLED_SET_COL_ADDR, 0x00, 0x7F};
    // ssd1306_write(device, SSD1306_WRITE_COMMAND, init_col, sizeof(init_col));

    // // Indicates start page and end page
    // uint8_t init_page[] = {OLED_SET_PAGE_ADDR, 0x00, 0x07};
    // ssd1306_write(device, SSD1306_WRITE_COMMAND, init_page, sizeof(init_page));
}

void ssd1306_fill_screen(ssd1306_spi_device *device, uint8_t byte)
{
    uint8_t buffer[OLED_NUM_PAGES][128];
}

void ssd1306_set_addressing(ssd1306_spi_device *device, uint8_t mode)
{
    uint8_t init_addressing[2] = {OLED_SET_MEM_ADDR, mode};
    ssd1306_write(device, SSD1306_WRITE_COMMAND, init_addressing, sizeof(init_addressing));
}
