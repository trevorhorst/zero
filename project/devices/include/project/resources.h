#ifndef DEVICES_RESOURCES_H
#define DEVICES_RESOURCES_H

#include <stdlib.h>
#include <string.h>

extern char sarah_bmp[];
extern const unsigned int sarah_bmp_size;

char *resources_load(char *resource, unsigned int size);
void resources_unload(char *resource );


#endif // DEVICES_RESOURCES_H