#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H
#include <fstream>
#include <pthread.h>
#include <string>
#include <stdarg.h>

#define SHOULD_PRINT 1
#define MODE 0

#if SHOULD_PRINT == 1
#define OUTPUT(format, ...) outputHandler->writeToFile(format, __VA_ARGS__)
#else
#define OUTPUT(format, ...)
#endif

class OutputHandler
{
private:
    std::FILE *_fOutputFile;
    pthread_mutex_t _outputLock;

public:
    // Constructor which opens a file
    OutputHandler(const std::string &fileName);

    // Thread safe file write
    void writeToFile(const char *format, ...);
};

#endif