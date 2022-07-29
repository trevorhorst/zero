#ifndef RP2040_COMMAND_H
#define RP2040_COMMAND_H

#include <functional>
#include <vector>
#include <string.h>

#include "common/control/control.h"
#include "common/json/cjson.h"
#include "common/logger.h"
#include "common/types.h"

#define COMMAND_NAME_MAX_SIZE   1024

#define BIND_PARAMETER( X ) std::bind( X, this, std::placeholders::_1 )

class Command
{
public:
    enum Error : uint32_t {
        NONE    = 0
        , SYNTAX
        , CMD_INVALID
        , CMD_MISSING
        , CMD_FAILED
        , PARAM_INVALID
        , PARAM_MISSING
        , PARAM_OUT_OF_RANGE
        , PARAM_WRONG_TYPE
        , PARAM_ACCESS_DENIED
        , CTRL_OPERATION_FAILED
        , GENERIC
        , MAX
    };

    using ParameterCallback = std::function< int32_t (cJSON*) >;
    using ParameterMap      = Types::CharHashMap< ParameterCallback >;

    Command(const char *name);
    virtual ~Command();

    const char *name();

    int32_t applyName( const char *name );

    virtual int32_t call( cJSON *params, cJSON *response, cJSON *details ) = 0;
    virtual int32_t setup();
    virtual int32_t teardown();
    virtual int32_t addControlObject( Control *object );
    virtual int32_t processParameterMap( cJSON *parameters, cJSON *response
                                         , cJSON *details, const ParameterMap *map );
    virtual int32_t mutate( cJSON *parameters, cJSON *response, cJSON *details );
    virtual int32_t access( cJSON *response, cJSON *details );

protected:
    std::vector< Control* > mControlObjects;

    const ParameterMap *requiredMap();
    const ParameterMap *optionalMap();
    const ParameterMap *mutableMap();
    const ParameterMap *accessableMap();

    ParameterMap mRequiredMap;
    ParameterMap mOptionalMap;
    ParameterMap mMutableMap;
    ParameterMap mAccessableMap;

private:
    char mName[ COMMAND_NAME_MAX_SIZE ];

};

#endif // RP2040_COMMAND_H
