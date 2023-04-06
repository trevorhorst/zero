#include <malloc.h>
#include <stdbool.h>

#include "project/game.h"

void conways_initialize(conways_game *game, uint32_t width, uint32_t height)
{
    uint32_t size = width * height;
    game->width = width;
    game->height = height;
    game->board = (uint8_t*)malloc(sizeof(uint8_t) * size);
    game->buffer = (uint8_t*)malloc(sizeof(uint8_t) * size);
    conways_reset(game);
}

void conways_deinitialize(conways_game *game)
{
    uint32_t size = 0;
    game->width = 0;
    game->height = 0;
    free(game->board);
    game->board = NULL;
    free(game->buffer);
    game->buffer = NULL;
}

void conways_reset(conways_game *game)
{
    game->generation = 0;
    for(uint32_t i = 0; i < (game->height * game->width); i++) {
        game->board[i] = rand() % 255;
        game->buffer[i] = 0;
    }
}

void conways_print_game_board(conways_game *game)
{
    uint32_t page_width_bits = 8;
    for(uint32_t column = 0; column < game->height; column++) {
        printf("%02d|", column);
        for(uint32_t page = 0; page < game->width; page++) {
            for(uint32_t shift = 0; shift < page_width_bits; shift++) {
                uint8_t bit = 1 << shift;
                uint32_t x = (column * game->width);
                uint32_t y = page;
                if(game->board[x + y] & bit) {
                    printf("o");
                } else {
                    printf(" ");
                }
            }
            printf("|");
        }
        printf("\n");
    }
}

void conways_print_game_buffer(conways_game *game)
{
    uint32_t page_width_bits = 8;
    for(uint32_t column = 0; column < game->height; column++) {
        printf("%02d|", column);
        for(uint32_t page = 0; page < game->width; page++) {
            for(uint32_t shift = 0; shift < page_width_bits; shift++) {
                uint8_t bit = 1 << shift;
                uint32_t x = (column * game->width);
                uint32_t y = page;
                if(game->buffer[x + y] & bit) {
                    printf("o");
                } else {
                    printf(" ");
                }
            }
            printf("|");
        }
        printf("\n");
    }
}

void conways_run_generation(conways_game *game, bool debug)
{
    int32_t page_width_bits = 8;
    for(int32_t column = 0; column < game->height; column++) {
        if(debug){ printf("%02d|", column); }
        for(int32_t page = 0; page < game->width; page++) {
            int32_t x = (column * game->width);
            int32_t y = page;
            game->buffer[x + y] = 0;
            for(int32_t shift = 0; shift < page_width_bits; shift++) {
                uint8_t bit = 1 << shift;

                int32_t neighbors = 0;
                bool left_valid = false;
                bool right_valid = false;
                bool up_valid = false;
                bool upper_page = false;
                bool down_valid = false;
                bool lower_page = false;

                // Check up of current bit
                if(column - 1 >= 0) {
                    neighbors += ((game->board[(x - game->width) + y] & bit) > 0);
                    up_valid = true;
                }

                // Check down of current bit
                if(column + 1 < game->height) {
                    neighbors += ((game->board[(x + game->width) + y] & bit) > 0);
                    down_valid = true;
                }

                // Check left of current bit
                if((shift + 1) < page_width_bits) {
                    // Bit is within the same page
                    neighbors += ((game->board[x + y] & (1 << (shift + 1))) > 0);
                    left_valid = true;
                } else if((page + 1) < game->width) {
                    // Bit is in a different page
                    neighbors += ((game->board[x + (y + 1)] & (1 << 0)) > 0);
                    left_valid = true;
                    upper_page = true;
                }

                // Check right of current bit
                if((shift - 1) >= 0) {
                    neighbors += ((game->board[x + y] & (1 << (shift - 1))) > 0);
                    right_valid = true;
                } else if((page - 1) >= 0) {
                    neighbors += ((game->board[x + (y - 1)] & (1 << (page_width_bits - 1))) > 0);
                    right_valid = true;
                    lower_page = true;
                }
            
                if(left_valid && up_valid) {
                    if(upper_page) {
                        neighbors += ((game->board[(x - game->width) + (y + 1)] & (1 << 0)) > 0);
                        // neighbors += ((ram[page - 1][column - 1] & (1 << (column_bits - 1))) > 0);
                    } else {
                        neighbors += ((game->board[(x - game->width) + y] & (1 << (shift + 1))) > 0);
                        // neighbors += ((ram[page][column - 1] & (1 << (shift - 1))) > 0);
                    }
                }

                if(right_valid && up_valid) {
                    if(lower_page) {
                        neighbors += ((game->board[(x - game->width) + (y - 1)] & (1 << (page_width_bits - 1))) > 0);
                        // neighbors += ((ram[page - 1][column - 1] & (1 << (column_bits - 1))) > 0);
                    } else {
                        neighbors += ((game->board[(x - game->width) + y] & (1 << (shift - 1))) > 0);
                        // neighbors += ((ram[page][column - 1] & (1 << (shift - 1))) > 0);
                    }
                }

                if(left_valid && down_valid) {
                    if(upper_page) {
                        neighbors += ((game->board[(x + game->width) + (y + 1)] & (1 << 0)) > 0);
                        // neighbors += ((ram[page - 1][column - 1] & (1 << (column_bits - 1))) > 0);
                    } else {
                        neighbors += ((game->board[(x + game->width) + y] & (1 << (shift + 1))) > 0);
                        // neighbors += ((ram[page][column - 1] & (1 << (shift - 1))) > 0);
                    }
                }

                if(right_valid && down_valid) {
                    if(lower_page) {
                        neighbors += ((game->board[(x + game->width) + (y - 1)] & (1 << (page_width_bits - 1))) > 0);
                        // neighbors += ((ram[page - 1][column - 1] & (1 << (column_bits - 1))) > 0);
                    } else {
                        neighbors += ((game->board[(x + game->width) + y] & (1 << (shift - 1))) > 0);
                        // neighbors += ((ram[page][column - 1] & (1 << (shift - 1))) > 0);
                    }
                }

                if(debug){ printf("%d", neighbors); }
                if(neighbors == 2) {
                    // Cell survives
                    game->buffer[x + y] |= (game->board[x + y] & bit);
                } else if(neighbors == 3) {
                    // Cell survives / becomes living
                    game->buffer[x + y] |= bit;
                } else {
                    // Cell dies
                    game->buffer[x + y] &= ~bit;
                }
            }
            if(debug){ printf("|"); }
        }
        if(debug){ printf("\n"); }
    }

    game->generation++;
}

void conways_run(conways_game *game)
{
    // SSD1306::DisplayRamWrite ram[2];
    // conways_display->fill_display_random(ram[0].ram);
    // conways_display->write_buffer(ram[0]);

    // do {
        // if(reset) {
        //     conways_display->fill_display_random(ram[0].ram);
        //     conways_display->write_buffer(ram[0]);

        //     gen = 0;
        //     cur = &ram[0];
        //     nxt = &ram[1];
        //     reset = false;
        // }

        // Simulate the new generation
        // printf("Generation: %d\n", gen++);
        conways_run_generation(game, false);


        // // Print the current generation and the board
        // conways_display->write_buffer(*nxt);
        // conways_print_game_board(game);

        // // Next board becomes the current and the current becomes the container for
        // // our next generation
        // SSD1306::DisplayRamWrite *temp = cur;
        // cur = nxt;
        // nxt = temp;
        // conways_display->fill_display(nxt->ram);
        uint8_t *temp = game->board;
        game->board = game->buffer;
        game->buffer = temp;


        // // Sleep so the results are easily viewable
        // sleep_us(speed);
    // } while(true);
}