#include "project/resources.h"

/**
 * @brief Loads a resource file
 * @param resource Resource file to load
 * @param size Size, in bytes, of the resource we are loading
 * @return Pointer to the resource
 */
char *resources_load(char *resource, unsigned int size)
{
    char *r = (char*)malloc(size + 1);
    memcpy(r, resource, size);
    r[size] = '\0';
    return r;
}


/**
 * @brief Unloads a resource file
 * @param resource Resource file to unload
 */
void resources_unload(char *resource)
{
    free(resource);
}