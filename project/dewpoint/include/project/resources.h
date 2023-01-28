#ifndef DEWPOINT_RESOURCES_H
#define DEWPOINT_RESOURCES_H

#include <stdlib.h>
#include <string.h>

#define SYMBOL_LENGTH 5

extern char font_txt[];
extern const unsigned int font_txt_size;

const char *getSymbol(char symbol);

char *resources_load(char *resource, unsigned int size);
void resources_unload(char *resource );


#endif // DEWPOINT_RESOURCES_H