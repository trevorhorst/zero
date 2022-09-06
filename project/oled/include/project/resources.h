#ifndef POKEDEX_RESOURCES_H
#define POKEDEX_RESOURCES_H

#include <stdlib.h>
#include <string.h>

extern "C" char red_blue_bmp[];
extern "C" const unsigned int red_blue_bmp_size;

// extern "C" char red_blue_font_bmp[];
// extern "C" const unsigned int red_blue_font_bmp_size;

char *resources_load(char *resource, unsigned int size);
void resources_unload(char *resource );


#endif // POKEDEX_RESOURCES_H