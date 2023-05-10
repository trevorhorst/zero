#ifndef SSD1351_H
#define SSD1351_H

#include <stdint.h>

#include "hardware/spi.h"
#include "hardware/gpio.h"

#define SSD1351_SET_COL ADDR        _u(0x15)
#define SSD1351_SET_ROW_ADDR        _u(0x75)
#define SSD1351_WRITE_RAM           _u(0x5C)
#define SSD1351_READ_RAM            _u(0x5D)
#define SSD1351_SET_REMAP           _u(0xA0)
#define SSD1351_SET_DISP_START_LINE _u(0xA1)
#define SSD1351_SET_DISP_OFFSET     _u(0xA2)
#define SSD1351_SET_DISP_MODE       _u(0xA4)
#define SSD1351_SET_FUN_SEL         _u(0xAB)
#define SSD1351_SET_SLP_MODE        _u(0xAE)
#define SSD1351_SET_PHASE_LEN       _u(0xB1)
#define SSD1351_DISP_ENHANCE        _u(0xB2)
#define SSD1351_SET_CLK_DIV         _u(0xB3)
#define SSD1351_SET_VSL             _u(0xB4)
#define SSD1351_SET_GPIO            _u(0xB5)
#define SSD1351_SET_PRECHRG_PER     _u(0xB6)
#define SSD1351_LUT_GS_PWM          _u(0xB8)
#define SSD1351_USE_LINEAR_LUT      _u(0xB9)
#define SSD1351_SET_PRECHRG_VOLT    _u(0xBB)
#define SSD1351_SET_VCOMH           _u(0xBE)
#define SSD1351_SET_CONTRAST_ABC    _u(0xC1)
#define SSD1351_SET_CONTRAST_MASTER _u(0xC7)
#define SSD1351_SET_MUX_RATIO       _u(0xCA)
// Command Lock
#define SSD1351_SET_CMD_LOCK        _u(0xFD)
#define SSD1351_CMD_LOCK_LOCK       _u(0x12)
#define SSD1351_CMD_LOCK_UNLOCK     _u(0x16)

typedef enum ssd1351_write_type_t {
    SSD1351_COMMAND     = 0,
    SSD1351_DATA        = 1
} ssd1351_write_type;

typedef enum ssd1351_sleep_mode_t {
    SSD1351_SLEEP_MODE_ON   = 0,
    SSD1351_SLEEP_MODE_OFF  = 1,
} ssd1351_sleep_mode;

typedef enum ssd1351_display_mode_t {
    SSD1351_DISPLAY_MODE_ALL_OFF    = 0,
    SSD1351_DISPLAY_MODE_ALL_ON     = 1,
    SSD1351_DISPLAY_MODE_RESET      = 2,
    SSD1351_DISPLAY_MODE_INVERSE    = 3
} ssd1351_display_mode;

typedef enum ssd1351_gpio_t {
    SSD1351_GPIO_HIZ_INPUT_DISABLED = 0,
    SSD1351_GPIO_HIZ_INPUT_ENABLED  = 1,
    SSD1351_GPIO_OUTPUT_LOW         = 2,
    SSD1351_GPIO_OUTPUT_HIGH        = 3,
} ssd1351_gpio;

typedef enum ssd1351_function_select_t {
    SSD1351_FUNCTION_SELECT_DISABLE_INTERNAL_V   = 0,
    SSD1351_FUNCTION_SELECT_ENABLE_INTERNAL_V    = 1,
    SSD1351_FUNCTION_SELECT_8_BIT_PARALLEL       = (0 << 6),
    SSD1351_FUNCTION_SELECT_16_BIT_PARALLEL      = (1 << 6),
    SSD1351_FUNCTION_SELECT_18_BIT_PARALLEL      = (2 << 6)
} ssd1351_function_select;

typedef struct ssd1351_spi_device_t {
    spi_inst_t *bus;
    uint32_t cs;
    uint32_t dc;
    uint32_t reset;
} ssd1351_spi_device;

int32_t ssd1351_spi_write(ssd1351_write_type type, ssd1351_spi_device *device,
                          const uint8_t *buffer, size_t buffer_length);
int32_t ssd1351_spi_write_command(ssd1351_spi_device *device,
                          const uint8_t *buffer, size_t buffer_length);
int32_t ssd1351_spi_write_data(ssd1351_spi_device *device,
                          const uint8_t *buffer, size_t buffer_length);
int32_t ssd1351_spi_write_display(ssd1351_spi_device *device,
                          const uint8_t *buffer, size_t buffer_length);

void ssd1351_spi_initialize_device(ssd1351_spi_device *device);
void ssd1351_spi_enable_display(ssd1351_spi_device *device, uint8_t enable);
void ssd1351_spi_reset_device(ssd1351_spi_device *device);
void ssd1351_spi_set_display_mode(ssd1351_spi_device *device, ssd1351_display_mode mode);
void ssd1351_spi_set_command_lock(ssd1351_spi_device *device, uint8_t lock);
void ssd1351_spi_set_clock_divider(ssd1351_spi_device *device, uint8_t div);
void ssd1351_spi_set_mux_ratio(ssd1351_spi_device *device, uint8_t mux);
void ssd1351_spi_set_display_offset(ssd1351_spi_device *device, uint8_t offset);
void ssd1351_spi_set_gpio(ssd1351_spi_device *device, ssd1351_gpio gpio0, ssd1351_gpio gpio1);
void ssd1351_spi_set_function_select(ssd1351_spi_device *device, uint8_t function);
void ssd1351_spi_set_phase_length(ssd1351_spi_device *device, uint8_t phase);
void ssd1351_spi_set_contrast(ssd1351_spi_device *device, uint8_t a, uint8_t b, uint8_t c);
void ssd1351_spi_set_contrast_master(ssd1351_spi_device *device, uint8_t contrast);
void ssd1351_spi_set_svl(ssd1351_spi_device *device);
void ssd1351_spi_set_vcomh(ssd1351_spi_device *device, uint8_t vcomh);
void ssd1351_spi_set_precharge_period(ssd1351_spi_device *device, uint8_t period);
void ssd1351_spi_set_sleep_mode(ssd1351_spi_device *device, ssd1351_sleep_mode mode);
void ssd1351_spi_set_write_ram(ssd1351_spi_device *device);


#endif // SSD1351_H
