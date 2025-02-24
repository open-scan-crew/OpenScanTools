#include "utils/ArgParser.h"
#include "utils/Logger.h"

void ArgParser::log(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        Logger::log(LoggerMode::LogConfig) << "arg[" << i << "] : [" << argv[i] << "]" << LOGENDL;
    }
}

bool ArgParser::find(int argc, char** argv, const char* opt)
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], opt) == 0)
            return true;
    }
    return false;
}

char* ArgParser::getArg(int argc, char** argv, const char* ext)
{
    for (int i = 0; i < argc; i++)
    {
        if (strstr(argv[i], ext) != NULL)
            return argv[i];
    }
    return nullptr;
}