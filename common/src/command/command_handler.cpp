#include "common/command/command_handler.h"

/**
 * @brief CommandHandler::CommandHandler Constructor
 */
CommandHandler::CommandHandler()
{

}

/**
 * @brief CommandHandler::map Retrieves a pointer to the command map
 * @return CharHashMap pointer to command map
 */
const Types::CharHashMap< Command* > *CommandHandler::map()
{
    return &mCommandMap;
}

/**
 * @brief CommandHandler::sortedMap Retrieves a sorted command map
 * @return CharMap sorted command map
 */
const Types::CharMap< Command* > CommandHandler::sortedMap()
{
    Types::CharMap< Command* > sorted( mCommandMap.begin(), mCommandMap.end( ) );
    return sorted;
}

/**
 * @brief CommandHandler::process Processes incoming command data
 * @param data Incoming data
 * @return Command::Error code
 */
int32_t CommandHandler::process( const char *data , char **response )
{

    /// @note Trying to emulate this pattern:
    /// {"cmd": "qrspddc", "error": {"code": 4, "details": "Command 'qrspddc' failed.", "type": "Command Failed"}, "msg": 1168563323, "rsp": 21, "success": false}

    int32_t error = Command::Error::NONE;

    // Container to capture responseJson data
    cJSON *responseJson = cJSON_CreateObject();
    cJSON *details = cJSON_CreateObject();
    cJSON *results = cJSON_CreateObject();


    // Parse the input data
    cJSON *parsedData = cJSON_Parse( data );
    cJSON *cmd = nullptr;

    // Check validity of the json data
    if( parsedData == nullptr ) {
        // We were unable to parse the data
        LOG_WARN( "Malformed JSON pattern" );
        error = Command::Error::SYNTAX;
        cJSON_AddNumberToObject( details, "code", error );
        // cJSON_AddStringToObject( details, "type", Command::Error::toString( error ) );
        // cJSON_AddStringToObject( details, "details", "Malformed JSON pattern" );
    } else {
        // Check the json data for a command entry
        cmd = cJSON_GetObjectItem( parsedData, COMMAND_NAME_CMD );

        if( !cJSON_IsString( cmd ) ) {
            // The command is not a string or there is no cmd parameter
            LOG_WARN( "Unable to process input: %s", data );
            error = Command::Error::CMD_MISSING;
            cJSON_AddNumberToObject( details, "code", error );
            // cJSON_AddStringToObject( details, "type", Command::Error::toString( error ) );
            // cJSON_AddStringToObject( details, "details", "Command identifier not found" );
        } else {
            // The command is valid and we can attempt to execute it

            // Start forming the response by adding the command name
            cJSON_AddStringToObject( responseJson, COMMAND_NAME_CMD, cmd->valuestring );

            // Check if the command exists in our map
            auto it = mCommandMap.find( cmd->valuestring );
            if( it == map()->end() ) {
                // The command does not exist
                error = Command::Error::CMD_INVALID;
                cJSON_AddNumberToObject( details, "code", error );
                // cJSON_AddStringToObject( details, "type", Command::Error::toString( error ) );
                // cJSON_AddStringToObject( details, "details", cmd->valuestring );
            } else {
                // Check the json data for a parameter entry
                cJSON *params = cJSON_GetObjectItem( parsedData, COMMAND_NAME_PARAMS );
                error = it->second->call( params, results, details );
            }

            if( error ) {
                // Add the error object to the response json
                cJSON_AddItemToObject( responseJson, "error", details );
                // Delete the unused results
                cJSON_Delete( results );
            } else {
                // Add the results object to the response json
                cJSON_AddItemToObject( responseJson, "results", results );
                // Delete the unused error details
                cJSON_Delete( details );
            }
        }
    }

    cJSON_AddBoolToObject( responseJson, COMMAND_NAME_SUCCESS, error ? false : true );

    // This will need to be cleaned up at the top level
    *response = cJSON_PrintUnformatted( responseJson );

    // Clean up our parsed json data
    cJSON_Delete( parsedData );
    cJSON_Delete( responseJson );

    return error;
}

/**
 * @brief CommandHandler::addCommand Add a new command
 * @param command Pointer to desired command
 */
void CommandHandler::addCommand( Command *command )
{
    if( command ) {
        // Add the command, if it exists
        mCommandMap[ command->name() ] = command;
    }
}
