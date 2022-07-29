#ifndef COMMON_COMMAND_HELP_H
#define COMMON_COMMAND_HELP_H

#include "common/command/command_template.h"
#include "common/command/command_handler.h"

#define COMMAND_NAME_HELP   "help"
#define COMMAND_NAME_MAP    "map"

class CommandHelp
        : public CommandTemplate< CommandHandler >
{
public:
    CommandHelp();

    int32_t map( cJSON *json );

protected:
    int32_t setup() override;
};

#endif // FAWKES_COMMAND_HELP_H
