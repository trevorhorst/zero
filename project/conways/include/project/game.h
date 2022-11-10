#ifndef CONWAYS_GAME_H
#define CONWAYS_GAME_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct conways_game_t {
    uint8_t *board;
    uint8_t *buffer;
    uint32_t width;
    uint32_t height;
    uint32_t generation;
} conways_game;

/**
 * @brief Initializes an instance of the game
 * 
 * @param game Instance of game 
 * @param width Width of the game board (in bytes)
 * @param height Height of the gameboard (in bytes)
 */
void conways_initialize(conways_game *game, uint32_t width, uint32_t height);

/**
 * @brief Deinitializes an instance of the game
 * 
 * @param game Instance of game
 */
void conways_deinitialize(conways_game *game);

/**
 * @brief Advances the game a single generation
 * 
 * @param game Instance of game 
 * @param debug Flag for additional debug statements
 */
void conways_run_generation(conways_game *game, bool debug);

/**
 * @brief Resets a game to an initial, randomized board
 * 
 * @param game Instance of game
 */
void conways_reset(conways_game *game);

/**
 * @brief Prints the game board to console
 * 
 * @param game Instance of game
 */
void conways_print_game_board(conways_game *game);

/**
 * @brief Prints the game buffer to console
 * 
 * @param game 
 */
void conways_print_game_buffer(conways_game *game);

void conways_run();

#endif // RP2040_CONTROL_GAME_H
