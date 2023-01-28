#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "core/command/command.h"

#define CMD_PARAM_TRUE    "true"
#define CMD_PARAM_FALSE   "false"

int32_t param_to_bool(char *value, bool *result)
{
    int32_t error = -1;
    bool r = false;
    if(value) {
        if(strncmp(value, CMD_PARAM_TRUE, strlen(value)) == 0) {
            r = true;
            error = 0;
        } else if(strncmp(value, CMD_PARAM_FALSE, strlen(value)) == 0) {
            r = false;
            error = 0;
        }
    }

    if(result) {
        *result = r;
    }

    return error;
}

int32_t param_to_number(char *value, double *result)
{
    int32_t error = -1;
    double r = 0.;
    if(value) {
        char *end = NULL;
        r = strtod(value, &end);
        if(r == 0.) {
            if(errno == ERANGE) {
                error = -1;
            }
        } else {
            error = 0;
        }
    }

    if(result) {
        *result = r;
    }

    return error;
}