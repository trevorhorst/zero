#include <string.h>
#include <stdio.h>

#include "pico/stdlib.h"

#include "common/console/console.h"
#include "common/logger.h"

#define CONSOLE_INPUT_LENGTH_MAX   1024

char console_input_buffer[CONSOLE_INPUT_LENGTH_MAX];
int console_cursor = 0;

static CommandHandler *cmd_handler = nullptr;

void console_set_command_handler(CommandHandler *handler)
{
    cmd_handler = handler;
}

/**
 * @brief Flush the consoles inpuut buffer and cursor
 */
void console_flush_input()
{
    memset(console_input_buffer, '\0', sizeof(console_input_buffer));
    console_cursor = 0;
}

std::vector<std::string> console_tokenize(char *input, const char *delimiter)
{
    std::string str( input );
    std::string token;
    std::vector< std::string > tokenized;
    size_t last = 0;
    size_t next = 0;

    // Split the input into segments by the delimeter
    while( ( next = str.find( delimiter, last ) ) != std::string::npos ) {
        if( next - last != 0 ) {
            // There is data in the string
            token = str.substr( last, next - last );
            tokenized.push_back( token );
        } else {
            // If the next and last values are the same, the string is empty
            // and we can ignore it
        }
        last = next + 1;
    }


    if( !str.substr( last ).empty() ) {
        // Need to place the the final substring in the list, unless it's empty
        tokenized.push_back( str.substr( last ) );
    }

    return tokenized;

}

void console_evaluate(char *input, const size_t &length)
{
    // Tokenize the input
    std::vector< std::string > tokenized = console_tokenize(input, " ");

    if( tokenized.empty() ) {
        // The input is empty
        // puts("");
    } else if( tokenized.at( 0 ) == "quit" ) {
        // The command is to quit, so lets quit. Nothing fancy here.
        // console->quit();
    } else {
        // Add the cmd string to the object
        cJSON *msg    = cJSON_CreateObject();
        cJSON *params = cJSON_CreateObject();
        cJSON *cmd    = nullptr;

        for( auto it = tokenized.begin(); it != tokenized.end(); it++ ) {
            if( *it == tokenized.at( 0 ) ) {
                // Add the command parameter
                cmd = cJSON_CreateString( (*it).c_str() );
            } else {
                auto t = it;
                it++;
                if( it == tokenized.end() ) {
                    LOG_WARN( "Parameter mismatch\n" );
                    return;
                } else {
                    // Parse the parameter
                    cJSON *param = cJSON_Parse( (*it).c_str() );
                    // Add the item to the parameter list
                    cJSON_AddItemToObject( params, (*t).c_str(), param );
                }
            }
        }

        cJSON_AddItemToObject( msg, "cmd", cmd );
        cJSON_AddItemToObject( msg, "params", params );

        char *msgStr = cJSON_PrintUnformatted( msg );
        // char msgRsp[ 1024 ] = { 0 };
        char *msgRsp = new char[1024];

        cmd_handler->process(msgStr, &msgRsp);
        printf("%s\n", msgRsp);
        // if( console->client() ) {
        //     console->client()->send( msgStr, msgRsp, 1024 );
        //     cJSON *rsp = cJSON_Parse( msgRsp );
        //     if( rsp ) {
        //         char *str = cJSON_Print( rsp );
        //         printf( "%s\n", str );
        //         cJSON_free( str );
        //         // cJSON *results = cJSON_GetObjectItem( rsp, "results" );
        //         // if( results ) {
        //         //     char *str = cJSON_Print( results );
        //         //     printf( "%s\n", str );
        //         //     cJSON_free( str );
        //         // } else {
        //         //     cJSON *error = cJSON_GetObjectItem( rsp, "error" );
        //         //     if( error ) {
        //         //         char *str = cJSON_Print( error );
        //         //         printf( "%s\n", str );
        //         //         cJSON_free( str );
        //         //     }
        //         // }
        //     }
        //     cJSON_Delete( rsp );

        // } else {
        //     LOG_INFO( "No client available" );
        // }

        delete[] msgRsp;

        cJSON_free( msgStr );

        // Clean up the message, this should delete all the components
        cJSON_Delete( msg );

    }
}

/**
 * @brief Runs the console loop
 */
void console_run()
{
    console_flush_input();

    printf(">");

    bool sequence = false;
    uint32_t sequenceCount = 0;

    while(true) {
        /// @note For some reason, 50 us was giving me issues. 100 us appears
        /// to be working though.
        int input = getchar_timeout_us(100);
        if(input == PICO_ERROR_TIMEOUT) {
            // This is okay, just loop back around
        // } else if(input == '\x1b' || sequence == true) {
        //     // if(sequenceCount >= 2) {
        //     //     // LOG_TRACE("Arrow Key\n");
        //     //     sequence = false;
        //     //     sequenceCount = 0;
        //     // } else {
        //     //     sequence = true;
        //     //     sequenceCount++;
        //     // }
        } else if(input == '\r') {
            // Evaluate the input string
            printf("\r>%s\n", console_input_buffer);
            console_input_buffer[console_cursor] = 0;
            console_evaluate(console_input_buffer, sizeof(console_input_buffer));
            // Once the input has been handled, flush the container
            console_flush_input();
            printf(">");
        } else if(input == '\b') {
            if(console_cursor > 0) {
                console_cursor--;
                console_input_buffer[console_cursor] = '\0';
                printf("\r>%s ", console_input_buffer);
                printf("\r>%s", console_input_buffer);
            }
        } else {
            if(console_cursor < CONSOLE_INPUT_LENGTH_MAX) {
                console_input_buffer[console_cursor] = input;
                console_cursor++;
                printf("\r>%s", console_input_buffer);
            }
        }
    }

}

