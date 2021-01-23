#ifndef MEM_MANAGER_H
#define MEM_MANAGER_H
#include <queue>
#include <vector>
#include <list>
#include <pthread.h>
#include <assert.h>
#include "process.h"
#include "page_table.h"

class MemManager
{
private:
    int _frameSize;

    // Memory array
    char *_physicalMemory;
    pthread_mutex_t _physicalMemoryLock;
    int _memoryCapacity;

    // Locks the page table
    std::vector<int> _freeFrames;
    pthread_mutex_t _freeFramesListLock;

    // Stores the occupied frames
    std::list<PageTableEntry *> _occupiedFrames;
    pthread_mutex_t _occupiedFramesListLock;
    std::list<PageTableEntry *>::iterator _clockIndex;

    // Stores the page tables
    std::vector<PageTable> _pageTables;
    pthread_mutex_t _pageTableLock;

public:
    // MemManager constructor
    MemManager(const int &memoryCapacity, const int &frameSize);

    // Creates a page table for the calling process
    void createPageTable(const int &pid, const int &memCapacity);

    // Finds page table with the associated pid
    PageTable *getPageTable(const int &pid);

    void tickClockAlgorithm(const int &pid);

    // Returns the frame number of an unoccupied frame and removes it from the free frames list
    // If no frames are available, run clock algorythm until one is evicted and return it
    int getFreeFrame(const int &pid);

    // Reads an int from virtual memory
    int read(const int &readAddressVirtual, const int &pid);

    // Writes an int to virtual memory
    void write(const int &data, const int &readAddressVirtual, const int &pid);
};

#endif