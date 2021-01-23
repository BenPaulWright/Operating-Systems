#include "process.h"
#include "mem_manager.h"
#include "output_handler.h"

// Manager used by all processes
extern MemManager *memManager;
extern OutputHandler *outputHandler;

Process::Process(const int &pid, const std::string &fileName, const int &pageSize)
{
    _pid = pid;
    _fileName = fileName;
    _pageSize = pageSize;
};

void *Process::staticRun(void *proc)
{
    Process *procInternal = reinterpret_cast<Process *>(proc);
    procInternal->runInternal();
    return nullptr;
}

int Process::run()
{
    return pthread_create(&_thread, NULL, &Process::staticRun, this);
}

int Process::join()
{
    return pthread_join(_thread, NULL);
}

void Process::runInternal()
{
    OUTPUT("Process %d started\n", _pid);
    std::string instructionString;
    std::ifstream fInputStream(_fileName);

    // Gets the mem capacity of the process
    std::getline(fInputStream, instructionString);
    _memCapacity = std::stoi(instructionString);

    // Tells the memory manager to allocate a page table for the process
    memManager->createPageTable(_pid, _memCapacity);

    // Temp variables to hold instruction data
    char instructionType;
    int registerIndex;
    int vMemoryAddress;

    while (std::getline(fInputStream, instructionString))
    {
        // Get instruction
        std::sscanf(instructionString.c_str(), "%c %*c%d %d", &instructionType, &registerIndex, &vMemoryAddress);

        // Make sure the requested address is within the bounds of virtual memory
        if (vMemoryAddress > _memCapacity)
        {
            printf("P%d: invalid request. memAddr: %d, memCap: %d\n", _pid, vMemoryAddress, _memCapacity);
            continue;
        }

        OUTPUT("P%d OPERATION: %c r%d %d\n", _pid, instructionType, registerIndex, vMemoryAddress);
        //outputHandler->writeToFile("P%d OPERATION: %c r%d %d\n", _pid, instructionType, registerIndex, vMemoryAddress);

        // Perform read/write
        if (instructionType == 'R')
            _registers[registerIndex].set(memManager->read(vMemoryAddress, _pid));
        else if (instructionType == 'W')
            memManager->write(_registers[registerIndex].get(), vMemoryAddress, _pid);
        OUTPUT("P%d: r%d = %#010x (mem at virtual address %#010x)\n", _pid, registerIndex, _registers[registerIndex].get(), vMemoryAddress);
    }

    fInputStream.close();
}