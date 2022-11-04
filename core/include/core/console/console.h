#ifndef CORE_CONSOLE_H
#define CORE_CONSOLE_H

#include <stdint.h>

struct console_command {
    uint32_t (*callback)();
};

void console_initialize();
void console_run();
void console_add_command(const char *key, struct console_command *container);

#endif // CORE_CONSOLE_H
