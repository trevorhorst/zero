#ifndef RP2040_CONSOLE_H
#define RP2040_CONSOLE_H

#include <string>
#include <string.h>
#include <vector>

#include "common/command/command_handler.h"

void console_set_command_handler(CommandHandler *handler);
void console_run();

#endif // RP2040_CONSOLE_H