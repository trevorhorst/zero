#ifndef CORE_CONSOLE_H
#define CORE_CONSOLE_H

#include <stdint.h>

#define CONSOLE_UNUSED_ARGS(AC, A)   (void)AC; (void)A;

struct console_command {
    int32_t (*callback)(int32_t, char **);
};

void console_initialize();
void console_run();
void console_add_command(const char *key, struct console_command *container);

#endif // CORE_CONSOLE_H
