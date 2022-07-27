#include "project/application.h"

Application::Application()
{
    stdio_init_all();
}

int32_t Application::run()
{
    int32_t error = 0;
    console_run();
    return 0;
}