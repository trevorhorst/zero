#ifndef CONWAYS_GAME_H
#define CONWAYS_GAME_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

// #include "common/drivers/ssd1306.h"

typedef struct conways_game_t {
    uint8_t *board;
    uint8_t *buffer;
    uint32_t width;
    uint32_t height;
    uint32_t generation;
} conways_game;

void print_ram_board(conways_game *game);
void print_ram_buffer(conways_game *game);
void step_ram_board(conways_game *game, bool debug);
void conways_initialize(conways_game *game, uint32_t width, uint32_t height);
void conways_reset(conways_game *game);
void conways_run();

// void conwaysSetDisplay(SSD1306 *display);
// void conwaysSetReset();
// void conwaysStepSpeed();
// 
// void printRamBoard(SSD1306::DisplayRam &ram);
// void checkRamBoard(SSD1306::DisplayRam &ram, SSD1306::DisplayRam &newRam, bool debug = false);
// void checkRamBoardNew(SSD1306::DisplayRam &ram, SSD1306::DisplayRam &newRam, bool debug = false);

#endif // RP2040_CONTROL_GAME_H
