#ifndef COMMON_COMMAND_HANDLER_H
#define COMMON_COMMAND_HANDLER_H

#include <vector>

#include "common/json/cjson.h"
#include "common/command/command.h"
#include "common/control/control_template.h"

#define COMMAND_NAME_CMD        "cmd"
#define COMMAND_NAME_PARAMS     "params"
#define COMMAND_NAME_RESULTS    "results"
#define COMMAND_NAME_SUCCESS    "success"

class CommandHandler
        : public ControlTemplate< CommandHandler >
{
public:
    CommandHandler();

    void addCommand( Command *command );
    int32_t process( const char *data, char **response );

    const Types::CharHashMap< Command* > *map();
    const Types::CharMap< Command* > sortedMap();

private:
    Types::CharHashMap< Command* > mCommandMap;
};

#endif // COMMON_COMMAND_HANDLER_H
