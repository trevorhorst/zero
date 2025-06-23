#include "core/drivers/ssd1351.h"

int32_t ssd1351_spi_write(ssd1351_write_type type, ssd1351_spi_device *device,
                          const uint8_t *buffer, size_t buffer_length)
{
    switch(type) {
    case SSD1351_COMMAND:
        gpio_put(device->dc, 0);
        break;
    case SSD1351_DATA:
        gpio_put(device->dc, 1);
        break;
    default:
        break;
    }

    // Take chip select low
    gpio_put(device->cs, 0);

    int32_t bytes = spi_write_blocking(device->bus, buffer, buffer_length);

    // Once the write is complete take chip select high
    gpio_put(device->cs, 1);

    return bytes;
}

int32_t ssd1351_spi_write_command(ssd1351_spi_device *device,
                          const uint8_t *buffer, size_t buffer_length)
{
    return ssd1351_spi_write(SSD1351_COMMAND, device, buffer, buffer_length);
}
int32_t ssd1351_spi_write_data(ssd1351_spi_device *device,
                          const uint8_t *buffer, size_t buffer_length)
{
    ssd1351_spi_set_write_ram(device);
    return ssd1351_spi_write(SSD1351_DATA, device, buffer, buffer_length);
}

int32_t ssd1351_spi_write_cmd(ssd1351_spi_device *device, uint8_t cmd,
                          const uint8_t *buffer, size_t buffer_length)
{
    gpio_put(device->dc, SSD1351_COMMAND);

    // Take chip select low
    gpio_put(device->cs, 0);

    // Write the command
    int32_t bytes = spi_write_blocking(device->bus, &cmd, 1);

    // Configure for data
    gpio_put(device->dc, SSD1351_DATA);

    bytes = spi_write_blocking(device->bus, buffer, buffer_length);

    // Take chip select low
    gpio_put(device->cs, 1);
}

void ssd1351_spi_initialize_device(ssd1351_spi_device *device)
{
    ssd1351_spi_reset_device(device);
    ssd1351_spi_set_display_mode(device, SSD1351_DISPLAY_MODE_ALL_OFF);
    ssd1351_spi_set_command_lock(device, SSD1351_CMD_LOCK_UNLOCK);
    ssd1351_spi_set_command_lock(device, 0xB1);
    // ssd1351_spi_set_clock_divider(device, 0xF1);
    ssd1351_spi_set_mux_ratio(device, 0x7F);
    ssd1351_spi_set_display_start_line(device, 0);
    ssd1351_spi_set_display_offset(device, 0);
    // ssd1351_spi_set_gpio(device, SSD1351_GPIO_HIZ_INPUT_DISABLED, SSD1351_GPIO_HIZ_INPUT_DISABLED);
    // ssd1351_spi_set_function_select(device, 0x01);
    // ssd1351_spi_set_phase_length(device, 0x32);
    // ssd1351_spi_set_vcomh(device, 0x05);
    ssd1351_spi_set_display_mode(device, SSD1351_DISPLAY_MODE_RESET);
    // ssd1351_spi_set_contrast(device, 0xC8, 0x80, 0xC8);
    // ssd1351_spi_set_contrast_master(device, 0x0F);
    // ssd1351_spi_set_svl(device);
    // ssd1351_spi_set_precharge_period(device, 0x01);
    ssd1351_spi_reset_cursor(device);
    ssd1351_spi_enable_display(device, true);
}

void ssd1351_spi_reset_device(ssd1351_spi_device *device)
{
    gpio_set_dir(device->reset, GPIO_OUT);
    /// @note Do we need this at all?
    // gpio_put(device->reset, 1);
    // sleep_ms(10);
    gpio_put(device->reset, 0);
    sleep_ms(10);
    gpio_put(device->reset, 1);
    sleep_ms(10);
}

void ssd1351_spi_enable_display(ssd1351_spi_device *device, uint8_t enable)
{
    if(enable) {
        ssd1351_spi_set_sleep_mode(device, SSD1351_SLEEP_MODE_OFF);
    } else {
        ssd1351_spi_set_sleep_mode(device, SSD1351_SLEEP_MODE_ON);
    }
}

void ssd1351_spi_reset_cursor(ssd1351_spi_device *device)
{
    ssd1351_spi_set_column_address(device, 0);
    ssd1351_spi_set_row_address(device, 0);
}

void ssd1351_spi_set_display_mode(ssd1351_spi_device *device, ssd1351_display_mode mode)
{
    uint8_t command[] = {(SSD1351_SET_DISP_MODE | mode)};
    ssd1351_spi_write_command(device, command, sizeof(command));
}

void ssd1351_spi_set_command_lock(ssd1351_spi_device *device, uint8_t lock)
{
    // uint8_t command[] = {SSD1351_SET_CMD_LOCK};
    // ssd1351_spi_write_command(device, command, sizeof(command));
    // uint8_t data[] = {lock};
    // ssd1351_spi_write_data(device, data, sizeof(data));
    ssd1351_spi_write_cmd(device, SSD1351_SET_CMD_LOCK, &lock, 1);
}

void ssd1351_spi_set_clock_divider(ssd1351_spi_device *device, uint8_t div)
{
    /// @note Locked by default
    uint8_t command[] = {SSD1351_SET_CLK_DIV, (div)};
    ssd1351_spi_write_command(device, command, sizeof(command));
}

void ssd1351_spi_set_mux_ratio(ssd1351_spi_device *device, uint8_t mux)
{
    // uint8_t command[] = {SSD1351_SET_MUX_RATIO, (mux)};
    // ssd1351_spi_write_command(device, command, sizeof(command));
    ssd1351_spi_write_cmd(device, SSD1351_SET_MUX_RATIO, &mux, 1);
}

void ssd1351_spi_set_display_offset(ssd1351_spi_device *device, uint8_t offset)
{
    // uint8_t command[] = {SSD1351_SET_DISP_OFFSET, (offset)};
    // ssd1351_spi_write_command(device, command, sizeof(command));
    ssd1351_spi_write_cmd(device, SSD1351_SET_DISP_OFFSET, &offset, 1);
}

void ssd1351_spi_set_gpio(ssd1351_spi_device *device, ssd1351_gpio gpio0, ssd1351_gpio gpio1)
{
    uint8_t command[] = {SSD1351_SET_GPIO, ((gpio0 & 0x3) | ((gpio1 & 0x3) << 2) )};
    ssd1351_spi_write_command(device, command, sizeof(command));
}

void ssd1351_spi_set_function_select(ssd1351_spi_device *device, uint8_t function)
{
    uint8_t command[] = {SSD1351_SET_FUN_SEL, function};
    ssd1351_spi_write_command(device, command, sizeof(command));
}

void ssd1351_spi_set_phase_length(ssd1351_spi_device *device, uint8_t phase)
{
    uint8_t command[] = {SSD1351_SET_PHASE_LEN, phase};
    ssd1351_spi_write_command(device, command, sizeof(command));
}

void ssd1351_spi_set_contrast(ssd1351_spi_device *device, uint8_t a, uint8_t b, uint8_t c)
{
    uint8_t data[] = {a, b, c};
    ssd1351_spi_write_cmd(device, SSD1351_SET_CONTRAST_ABC, data, sizeof(data));
}

void ssd1351_spi_set_contrast_master(ssd1351_spi_device *device, uint8_t contrast)
{
    ssd1351_spi_write_cmd(device, SSD1351_SET_CONTRAST_MASTER, &contrast, sizeof(contrast));
}

void ssd1351_spi_set_svl(ssd1351_spi_device *device)
{
    uint8_t command[] = {SSD1351_SET_VSL, 0xA0, 0xB5, 0x55};
    ssd1351_spi_write_command(device, command, sizeof(command));
}

void ssd1351_spi_set_vcomh(ssd1351_spi_device *device, uint8_t vcomh)
{
    uint8_t command[] = {SSD1351_SET_VCOMH, vcomh};
    ssd1351_spi_write_command(device, command, sizeof(command));
}

void ssd1351_spi_set_precharge_period(ssd1351_spi_device *device, uint8_t period)
{
    uint8_t command[] = {SSD1351_SET_PRECHRG_PER, period};
    ssd1351_spi_write_command(device, command, sizeof(command));

}

void ssd1351_spi_set_sleep_mode(ssd1351_spi_device *device, ssd1351_sleep_mode mode)
{
    uint8_t command[] = {(SSD1351_SET_SLP_MODE | mode)};
    ssd1351_spi_write_command(device, command, sizeof(command));
}

void ssd1351_spi_set_write_ram(ssd1351_spi_device *device)
{
    uint8_t command[] = {SSD1351_WRITE_RAM};
    ssd1351_spi_write_command(device, command, sizeof(command));
}

void ssd1351_spi_set_display_start_line(ssd1351_spi_device *device, uint8_t start)
{
    /// @note this works
    ssd1351_spi_write_cmd(device, SSD1351_SET_DISP_START_LINE, &start, 1);

    /// @note This does NOT work
    // uint8_t command[] = {SSD1351_SET_DISP_START_LINE};
    // ssd1351_spi_write_command(device, command, sizeof(command));
    // ssd1351_spi_write_data(device, &start, 1);
}

void ssd1351_spi_set_column_address(ssd1351_spi_device *device, uint8_t column)
{
    ssd1351_spi_write_cmd(device, SSD1351_SET_COL_ADDR, &column, sizeof(column));
}

void ssd1351_spi_set_row_address(ssd1351_spi_device *device, uint8_t row)
{
    ssd1351_spi_write_cmd(device, SSD1351_SET_ROW_ADDR, &row, sizeof(row));
}
