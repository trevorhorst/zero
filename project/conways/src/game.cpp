#include "project/game.h"

static SSD1306 *conways_display = nullptr;
static bool reset = false;
uint32_t speed = 0;

void conwaysSetDisplay(SSD1306 *display)
{
    conways_display = display;
}

void conwaysSetReset()
{
    reset = true;
}

void conwaysStepSpeed()
{
    static uint32_t step = 0;

    speed = 50000 * step;

    step++;

    if(step >= 5) {
        step = 0;
    }

}

void conwaysRun()
{
    SSD1306::DisplayRamWrite ram[2];
    conways_display->fill_display_random(ram[0].ram);
    conways_display->write_buffer(ram[0]);

    uint32_t gen = 0;
    SSD1306::DisplayRamWrite *cur = &ram[0];
    SSD1306::DisplayRamWrite *nxt = &ram[1];

    do {
        if(reset) {
            conways_display->fill_display_random(ram[0].ram);
            conways_display->write_buffer(ram[0]);

            gen = 0;
            cur = &ram[0];
            nxt = &ram[1];
            reset = false;
        }

        // Simulate the new generation
        printf("Generation: %d\n", gen++);
        checkRamBoard(cur->ram, nxt->ram, false);

        // Print the current generation and the board
        conways_display->write_buffer(*nxt);
        // printRamBoard(nxt->ram);

        // Next board becomes the current and the current becomes the container for
        // our next generation
        SSD1306::DisplayRamWrite *temp = cur;
        cur = nxt;
        nxt = temp;
        conways_display->fill_display(nxt->ram);

        // Sleep so the results are easily viewable
        sleep_us(speed);
    } while(true);


}

void printRamBoard(SSD1306::DisplayRam &ram)
{
    for(uint32_t page = 0; page < OLED_PAGE_HEIGHT; page++) {
        for(uint32_t shift = 0; shift < 8; shift++) {
            uint32_t bit = 1 << shift;
            for(uint32_t column = 0; column < OLED_WIDTH; column++) {
                if((ram[page][column] & bit) == 0) {
                    printf("  ");
                } else {
                    // printf("%01X ", ram[page][column] & 0xF);
                    printf("o ");
                }
            }
            printf("\n");
        }
    }
}

void checkRamBoardNew(SSD1306::DisplayRam &ram, SSD1306::DisplayRam &newRam, bool debug)
{
    for(int32_t page = 0; page < OLED_NUM_PAGES; page++) {
        for(int32_t shift = 0; shift < OLED_PAGE_HEIGHT; shift++) {
            for(int32_t column = 0; column < OLED_WIDTH; column++) {
            }
        }
    }
}

void checkRamBoard(SSD1306::DisplayRam &ram, SSD1306::DisplayRam &newRam, bool debug)
{
    int32_t column_bits = 8;
    for(int32_t page = 0; page < OLED_PAGE_HEIGHT; page++) {
        for(int32_t shift = 0; shift < column_bits; shift++) {
            uint8_t bit = 1 << shift;
            for(int32_t column = 0; column < OLED_WIDTH; column++) {
                int32_t neighbors = 0;
                bool leftValid = false;
                bool rightValid = false;
                bool upValid = false;
                bool upperPage = false;
                bool downValid = false;
                bool lowerPage = false;

                if((column - 1) >= 0) {
                    // Check left
                    neighbors += ((ram[page][column - 1] & bit) > 0);
                    leftValid = true;
                }

                if((column + 1) < OLED_WIDTH) {
                    // Check right
                    neighbors += ((ram[page][column + 1] & bit) > 0);
                    rightValid = true;
                }

                if((shift - 1) >= 0) {
                    // Check up
                    neighbors += ((ram[page][column] & (1 << (shift - 1))) > 0);
                    upValid = true;
                } else if((page - 1) >= 0) {
                    // If there is another page before this one, check the last
                    // bit in that column
                    neighbors += ((ram[page - 1][column] & (1 << (column_bits - 1))) > 0);
                    upValid = true;
                    upperPage = true;
                }

                if((shift + 1) < column_bits) {
                    // Check down
                    neighbors += ((ram[page][column] & (1 << (shift + 1))) > 0);
                    downValid = true;
                } else if((page + 1) < OLED_PAGE_HEIGHT) {
                    // If there is another page after this one, check the first
                    // bit in that column
                    neighbors += ((ram[page + 1][column] & (1 << 0)) > 0);
                    downValid = true;
                    lowerPage = true;
                }

                if(leftValid && upValid) {
                    if(upperPage) {
                        neighbors += ((ram[page - 1][column - 1] & (1 << (column_bits - 1))) > 0);
                    } else {
                        neighbors += ((ram[page][column - 1] & (1 << (shift - 1))) > 0);
                    }
                }

                if(rightValid && upValid) {
                    if(upperPage) {
                        neighbors += ((ram[page - 1][column + 1] & (1 << (column_bits - 1))) > 0);
                    } else {
                        neighbors += ((ram[page][column + 1] & (1 << (shift - 1))) > 0);
                    }
                }

                if(rightValid && downValid) {
                    if(lowerPage) {
                        neighbors += ((ram[page + 1][column + 1] & (1 << 0)) > 0);
                    } else {
                        neighbors += ((ram[page][column + 1] & (1 << (shift + 1))) > 0);
                    }
                }

                if(leftValid && downValid) {
                    if(lowerPage) {
                        neighbors += ((ram[page + 1][column - 1] & (1 << 0)) > 0);
                    } else {
                        neighbors += ((ram[page][column - 1] & (1 << (shift + 1))) > 0);
                    }
                }

                if(debug){ printf("%d", neighbors); }
                if(neighbors == 2) {
                    // Cell survives
                    newRam[page][column] |= (ram[page][column] & bit);
                } else if(neighbors == 3) {
                    // Cell survives / becomes living
                    newRam[page][column] |= bit;
                } else {
                    // Cell dies
                    newRam[page][column] &= ~bit;
                }
                if(debug){ printf("."); }
            }
            if(debug){ printf("\n"); }
        }
    }
}
