#include "common/command/command.h"

/**
 * @brief Command::Command Constructor
 * @param name Desired name of the command
 * @param type Type of command
 */
Command::Command( const char *name)
    : mName{ 0 }
{
    applyName( name );
}

Command::~Command()
{

}

const char *Command::name()
{
    return mName;
}

/**
 * @brief Command::requiredMap Returns a map of parameters that are required to
 *  properly handle a command
 * @return
 */
const Command::ParameterMap *Command::requiredMap()
{
    return &mRequiredMap;
}

/**
 * @brief Command::optionalMap Returns a map of parameters that are optional and
 *  may alter how a command is handled
 * @return
 */
const Command::ParameterMap *Command::optionalMap()
{
    return &mOptionalMap;
}

/**
 * @brief Command::mutableMap Returns a map of parameters that are mutable for a
 *  given command
 * @return
 */
const Command::ParameterMap *Command::mutableMap()
{
    return &mMutableMap;
}

/**
 * @brief Command::accessableMap Returns a map of parameters that are accessable
 *  for a given command
 * @return
 */
const Command::ParameterMap *Command::accessableMap()
{
    return &mAccessableMap;
}

/**
 * @brief Command::setup Can be overloaded for any setup the command may need
 * @return
 */
int32_t Command::setup()
{
    return Error::NONE;
}

/**
 * @brief Command::teardown Can be overloaded for any teardown the command may need
 * @return Error code
 */
int32_t Command::teardown()
{
    return Error::NONE;
}

int32_t Command::addControlObject( Control *object )
{
    if( object ) {
        mControlObjects.push_back( object );
    }
    return Error::NONE;
}

/**
 * @brief Command::processParameterMap Processes the information found in a
 *  parameter map
 * @param parameters Incoming parameters to process
 * @param response Results of processing the data
 * @param map
 * @return
 */
int32_t Command::processParameterMap( cJSON *parameters, cJSON *response
                                      , cJSON *details, const ParameterMap *map )
{
    int32_t error = Error::NONE;

    // Iterate over the required parameters
    for( auto it = map->begin()
         ; ( it != map->end() ) && ( error == Error::NONE )
         ; it++ ) {

        // Retrieve parameter information
        const char *parameterName = it->first;
        ParameterCallback parameterCallback = it->second;

        // Attempt to remove a parameter from the list received
        cJSON *parameterData = cJSON_DetachItemFromObject( parameters, parameterName );

        if( parameterData == nullptr ) {
            // The parameter was NOT found
            if( map == requiredMap() ) {
                // If the parameter map is required, report as an error
                LOG_DEBUG( "Required parameter missing: %s\n", parameterName );
                error = Error::PARAM_MISSING;
            }
        } else {
            // The parameter was found, perform callback to handle data and
            // propogate error
            error = parameterCallback( parameterData );
        }

        if( error ) {
            cJSON_AddNumberToObject( details, "code", error );
            // cJSON_AddStringToObject( details, "type", Error::toString( error ) );
            cJSON_AddStringToObject( details, "details", parameterName );
        }

        cJSON_Delete( parameterData );
    }

    return error;
}

int32_t Command::mutate( cJSON *parameters, cJSON *response, cJSON *details )
{
    int32_t error = Error::NONE;

    (void)response;

    const ParameterMap *map = mutableMap();

    // Iterate over the required parameters
    for( auto it = map->begin()
         ; ( it != map->end() ) && ( error == Error::NONE )
         ; it++ ) {

        // Retrieve parameter information
        const char *parameterName = it->first;
        ParameterCallback parameterCallback = it->second;

        // Attempt to remove a parameter from the list received
        cJSON *parameterData = cJSON_DetachItemFromObject( parameters, parameterName );

        if( parameterData == nullptr ) {
            // The parameter was NOT found, this is not an error
        } else {
            // The parameter was found, perform callback to handle data and
            // propogate error
            error = parameterCallback( parameterData );
        }

        if( error ) {
            cJSON_AddNumberToObject( details, "code", error );
            // cJSON_AddStringToObject( details, "type", Error::toString( error ) );
            cJSON_AddStringToObject( details, "details", parameterName );
        }

        cJSON_Delete( parameterData );
    }

    return error;
}

int32_t Command::access( cJSON *response, cJSON *details )
{
    int32_t error = Error::NONE;

    const ParameterMap *map = accessableMap();

    for( auto it = map->begin()
         ; ( it != map->end() ) && ( error == Error::NONE )
         ; it++ ) {

        const char *parameterName = it->first;
        ParameterCallback parameterCallback = it->second;
        error = parameterCallback( response );

        if( error ) {
            cJSON_AddNumberToObject( details, "code", error );
            // cJSON_AddStringToObject( details, "type", Error::toString( error ) );
            cJSON_AddStringToObject( details, "details", parameterName );
        }
    }

    return error;
}

int32_t Command::applyName( const char *name )
{
    int32_t error = Error::NONE;

    if( name != nullptr ) {
        strncpy( mName, name, COMMAND_NAME_MAX_SIZE - 1 );
        mName[ COMMAND_NAME_MAX_SIZE - 1 ] = '\0';
    } else {
        LOG_WARN( "Command name is invalid\n" );
        error = Error::CTRL_OPERATION_FAILED;
    }

    return error;
}

