#ifndef TOOLS_I2C_H
#define TOOLS_I2C_H

#include <stdint.h>

#include "hardware/i2c.h"

/**
 * @brief 
 * 
 * @param addr 
 * @return true 
 * @return false 
 */
bool tools_i2c_reserved_addr(uint8_t addr);

/**
 * @brief Scans a given I2C bus for valid devices attached to the bus and prints
 *  their address.
 * 
 * @param bus Desired I2C bus to scan
 */
void tools_i2c_bus_scan(i2c_inst_t *bus);

#endif // TOOLS_I2C_H