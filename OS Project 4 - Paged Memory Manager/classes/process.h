#ifndef PROCESS_H
#define PROCESS_H
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <memory>

// 32 bit big endian register
class Reg
{
private:
    // Internal byte array of length 4
    char *_regInternal;

public:
    Reg() { set(0); }

    // Returns the contents of reg_internal as an int
    int get() { return *(int *)_regInternal; };

    // Encodes an int into reg_internal in big endian
    void set(const int &integer) { _regInternal = (char *)(&integer); };
};

class Process
{
private:
    // Process ID
    int _pid = -1;
    // File name for reading process data
    std::string _fileName;
    // Virtual memory capacity
    int _memCapacity = -1;
    // Page size
    int _pageSize = -1;
    // Internal thread
    pthread_t _thread;
    // 32 int registers
    Reg _registers[32];

    // Static wrapper for the run function
    static void *staticRun(void *proc);

    // Internal function run on the internal thread
    void runInternal();

public:
    // Process constructor
    Process(const int &pid, const std::string &fileName, const int &pageSize);

    // Runs pthread_create for internal thread and returns the result
    int run();

    // Runs pthread_join for internal thread and returns the result
    int join();
};

#endif