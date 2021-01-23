#include "output_handler.h"

OutputHandler::OutputHandler(const std::string &fileName)
{
    _fOutputFile = fopen(fileName.c_str(), "w");
    pthread_mutex_init(&_outputLock, NULL);
}

void OutputHandler::writeToFile(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    pthread_mutex_lock(&_outputLock);
    if (MODE == 0)
        vfprintf(_fOutputFile, format, args);
    if (MODE == 1)
        vprintf(format, args);
    pthread_mutex_unlock(&_outputLock);
    va_end(args);
}