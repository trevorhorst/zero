#include <string.h>
#include <stdio.h>

#include "pico/stdlib.h"

#include "common/console/console.h"
#include "common/logger.h"

#define CONSOLE_INPUT_LENGTH_MAX   1024

char console_input_buffer[CONSOLE_INPUT_LENGTH_MAX];
int console_cursor = 0;

/**
 * @brief Flush the consoles inpuut buffer and cursor
 */
void console_flush_input()
{
    memset(console_input_buffer, '\0', sizeof(console_input_buffer));
    console_cursor = 0;
}

/**
 * @brief Evaluate the console input string
 * 
 * @param buffer Buffer containing the console input string
 * @param bufferSize Size of the console input string
 */
void console_evaluate(char *buffer, size_t bufferSize)
{
    LOG_TRACE("Evaluate: %s\n", buffer);
}

/**
 * @brief Runs the console loop
 */
void console_run()
{
    console_flush_input();

    printf(">");

    bool sequence = false;
    uint32_t sequenceCount = 0;

    while(true) {
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
            if(console_cursor < CONSOLE_INPUT_LENGTH_MAX) {
                console_input_buffer[console_cursor] = input;
                console_cursor++;
                printf("\r>%s", console_input_buffer);
            }
        }
    }

}