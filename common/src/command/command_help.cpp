#include "common/command/command_help.h"

CommandHelp::CommandHelp()
    : CommandTemplate< CommandHandler >(COMMAND_NAME_HELP)
{
    mAccessableMap[COMMAND_NAME_MAP] = BIND_PARAMETER( &CommandHelp::map );
}

int32_t CommandHelp::setup()
{
    mControlObject = static_cast< CommandHandler* >( mControlObjects.back() );
    return Error::NONE;
}

int32_t CommandHelp::map( cJSON *json )
{
    int32_t error = Error::NONE;

    const Types::CharMap< Command * > map = mControlObject->sortedMap();

    cJSON *array = cJSON_CreateArray();
    // cJSON_AddItemToObject( json, COMMAND_NAME_MAP, array );

    for( auto it = map.begin(); it != map.end(); it++ ) {
        cJSON *str = cJSON_CreateString( it->first );
        cJSON_AddItemToArray( array, str );
    }

    cJSON_AddItemToObject( json, "commands", array );

    return error;
}
