#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "pico/stdlib.h"

#include "core/console/console.h"
#include "core/logger.h"
#include "core/types/hash_table.h"

#define CONSOLE_INPUT_LENGTH_MAX   1024

char console_input_buffer[CONSOLE_INPUT_LENGTH_MAX];
int console_cursor = 0;

static struct hash_table command_map;

/**
 * @brief Flush the consoles inpuut buffer and cursor
 */
void console_flush_input()
{
    memset(console_input_buffer, '\0', sizeof(console_input_buffer));
    console_cursor = 0;
}

void console_evaluate(char *input, const size_t length)
{
    // LOG_INFO("evaluate: %s\n", input);
    struct console_command *cmd = (struct console_command*)hash_table_get(&command_map, input);
    if(cmd) {
        cmd->callback();
    }
}

void console_initialize()
{
    hash_table_initialize(&command_map, 10);
}

void console_run()
{
    printf(">");

    uint8_t sequence = 0;
    uint32_t sequenceCount = 0;

    while(1) {
        /// @note For some reason, 50 us was giving me issues. 100 us appears
        /// to be working though.
        int input = getchar_timeout_us(100);
        if(input == PICO_ERROR_TIMEOUT) {
            // This is okay, just loop back around
        // } else if(input == '\x1b' || sequence == true) {
        //     // if(sequenceCount >= 2) {
        //     //     // LOG_TRACE("Arrow Key\n");
        //     //     sequence = false;
        //     //     sequenceCount = 0;
        //     // } else {
        //     //     sequence = true;
        //     //     sequenceCount++;
        //     // }
        } else if(input == '\r') {
            // Evaluate the input string
            printf("\r>%s\n", console_input_buffer);
            console_input_buffer[console_cursor] = 0;
            console_evaluate(console_input_buffer, sizeof(console_input_buffer));
            // Once the input has been handled, flush the container
            console_flush_input();
            printf(">");
        } else if(input == '\b') {
            if(console_cursor > 0) {
                console_cursor--;
                console_input_buffer[console_cursor] = '\0';
                printf("\r>%s ", console_input_buffer);
                printf("\r>%s", console_input_buffer);
            }
        } else {
            // Add character to the buffer if there is room
            if(console_cursor < CONSOLE_INPUT_LENGTH_MAX) {
                console_input_buffer[console_cursor] = input;
                console_cursor++;
                // Redraw the buffer to screen
                printf("\r>%s", console_input_buffer);
            }
        }
    }
}

void console_add_command(const char *key, struct console_command *container)
{
    hash_table_insert(&command_map, key, (void*)container);
}
