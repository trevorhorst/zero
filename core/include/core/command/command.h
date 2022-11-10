#ifndef CORE_COMMAND_H
#define CORE_COMMAND_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Converts a char array to a bool value
 * 
 * @param value Char array to convert
 * @param result Container to store result
 * @return int32_t Error code, 0 indicates success
 */
int32_t param_to_bool(char *value, bool *result);

#endif // CORE_COMMAND_H