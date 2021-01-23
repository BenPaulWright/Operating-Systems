#include "mem_manager.h"
#include "output_handler.h"

extern OutputHandler *outputHandler;

#pragma region Mem Manager Functions

MemManager::MemManager(const int &memoryCapacity, const int &frameSize)
{
    _memoryCapacity = memoryCapacity;
    _frameSize = frameSize;

    // Malloc _physicalMemory and fill it with random values
    _physicalMemory = new char[_memoryCapacity];
    for (int i = 0; i < _memoryCapacity; i++)
        _physicalMemory[i] = rand();

    // At first, all frames are free
    _freeFrames.reserve(_memoryCapacity / _frameSize);
    for (int i = 0; i < _memoryCapacity / _frameSize; i++)
        _freeFrames.emplace_back(i);

    // Init mutexes
    pthread_mutex_init(&_physicalMemoryLock, NULL);
    pthread_mutex_init(&_pageTableLock, NULL);
    pthread_mutex_init(&_freeFramesListLock, NULL);
    pthread_mutex_init(&_occupiedFramesListLock, NULL);
}

PageTable *MemManager::getPageTable(const int &pid)
{
    PageTable *returnVal = nullptr;
    pthread_mutex_lock(&_pageTableLock);
    // Loops through _pageTables and finds the one which is assigned to the process
    // Should be fast: O(_pageTables.size())
    // Can't access by index in case the threads call createPageTable out of order
    // Also prevents out of bounds in case pids are not sequential starting at 0s
    for (int i = 0; i < _pageTables.size(); i++)
    {
        if (_pageTables[i].getAssociatedPid() == pid)
        {
            returnVal = &_pageTables[i];
            break;
        }
    }
    pthread_mutex_unlock(&_pageTableLock);
    return returnVal;
}

void MemManager::createPageTable(const int &pid, const int &memCapacity)
{
    pthread_mutex_lock(&_pageTableLock);
    // Create page table and put it in the vector
    _pageTables.push_back(PageTable(pid, _frameSize, memCapacity));
    _pageTables[_pageTables.size() - 1].AssertNotEmpty("MemManager::createPageTable(const int &pid, const int &memCapacity)");
    pthread_mutex_unlock(&_pageTableLock);
}

void MemManager::tickClockAlgorithm(const int &pid)
{
    pthread_mutex_lock(&_occupiedFramesListLock);

    if ((*_clockIndex)->checkReferenceBit())
    {
        // If the reference bit is zero
        // 1. add the entry to the free frames list
        // 2. remove it from the occupied frames list
        _freeFrames.push_back((*_clockIndex)->getFrameNumber());
        auto valToRemove = *_clockIndex;

        // Loop iterator
        if (_clockIndex == _occupiedFrames.end())
            _clockIndex = _occupiedFrames.begin();
        else
            std::advance(_clockIndex, 1);

        //int removedValPid = (*_clockIndex)->getPid();
        //PageTable *table = getPageTable(removedValPid);
        _occupiedFrames.remove(valToRemove);

        OUTPUT("P%d: evicting process %d page %d\n", pid, (*_clockIndex)->getPid(), (*_clockIndex)->getPageNumber());
    }
    else
        (*_clockIndex)->clearReferenceBit();
    pthread_mutex_unlock(&_occupiedFramesListLock);
}

int MemManager::getFreeFrame(const int &pid)
{
    if (_freeFrames.size() == 0)
        _clockIndex = _occupiedFrames.begin();
    while (_freeFrames.size() == 0)
        tickClockAlgorithm(pid);

    assert(_freeFrames.size() <= _memoryCapacity);
    int returnVal = _freeFrames[0];
    _freeFrames.erase(_freeFrames.begin());

    return returnVal;
}

int MemManager::read(const int &readAddressVirtual, const int &pid)
{
    // Grab the table and entry (thread safe page table vector access)
    PageTable *table = getPageTable(pid);
    PageTableEntry *entry = table->getEntry(readAddressVirtual);

    // If not valid, find a frame
    if (!entry->checkValidBit())
    {
        bool wasFreeFrameAvailable;
        OUTPUT("P%d: page %d not resident in memory\n", pid, table->getPageNumber(readAddressVirtual));
        pthread_mutex_lock(&_freeFramesListLock);
        wasFreeFrameAvailable = _freeFrames.size() > 0;
        entry->setFrameNumber(getFreeFrame(pid));
        pthread_mutex_unlock(&_freeFramesListLock);
        entry->setValidBit();
        // Add entry pointer to list of occupied frames
        _occupiedFrames.push_back(entry);
        if (wasFreeFrameAvailable)
            OUTPUT("P%d: using free frame %d\n", pid, table->getFrameNumber(readAddressVirtual));
        OUTPUT("P%d: new translation from page %d to frame %d\n", pid, table->getPageNumber(readAddressVirtual), table->getFrameNumber(readAddressVirtual));
    }
    else
        OUTPUT("P%d: valid translation from page %d to frame %d\n", pid, table->getPageNumber(readAddressVirtual), table->getFrameNumber(readAddressVirtual));

    // Grab the physical address
    int physicalAddress = table->getPhysicalAddress(readAddressVirtual);
    OUTPUT("P%d: translated VA 0x%08x to PA 0x%08x\n", table->getAssociatedPid(), readAddressVirtual, physicalAddress);

    // Read the data
    pthread_mutex_lock(&_physicalMemoryLock);
    int *returnVal = (int *)&_physicalMemory[physicalAddress];
    entry->setReferenceBit();
    pthread_mutex_unlock(&_physicalMemoryLock);
    return *returnVal;
}

void MemManager::write(const int &data, const int &readAddressVirtual, const int &pid)
{
    // Grab the table and entry (thread safe page table vector access)
    PageTable *table = getPageTable(pid);
    PageTableEntry *entry = table->getEntry(readAddressVirtual);

    // If not valid, find a frame
    if (!entry->checkValidBit())
    {
        bool wasFreeFrameAvailable;
        OUTPUT("P%d: page %d not resident in memory\n", pid, table->getPageNumber(readAddressVirtual));
        pthread_mutex_lock(&_freeFramesListLock);
        wasFreeFrameAvailable = _freeFrames.size() > 0;
        entry->setFrameNumber(getFreeFrame(pid));
        pthread_mutex_unlock(&_freeFramesListLock);
        entry->setValidBit();
        // Add entry pointer to list of occupied frames
        _occupiedFrames.push_back(entry);
        if (wasFreeFrameAvailable)
            OUTPUT("P%d: using free frame %d\n", pid, table->getFrameNumber(readAddressVirtual));
        OUTPUT("P%d: new translation from page %d to frame %d\n", pid, table->getPageNumber(readAddressVirtual), table->getFrameNumber(readAddressVirtual));
    }
    else
        OUTPUT("P%d: valid translation from page %d to frame %d\n", pid, table->getPageNumber(readAddressVirtual), table->getFrameNumber(readAddressVirtual));

    // Grab the physical address
    int physicalAddress = table->getPhysicalAddress(readAddressVirtual);
    OUTPUT("P%d: translated VA 0x%08x to PA 0x%08x\n", table->getAssociatedPid(), readAddressVirtual, physicalAddress);

    // If modified, mark changed and write data
    pthread_mutex_lock(&_physicalMemoryLock);
    if (data != _physicalMemory[physicalAddress])
    {
        _physicalMemory[physicalAddress] = data;
        entry->setReferenceBit();
        entry->setDirtyBit();
    }
    pthread_mutex_unlock(&_physicalMemoryLock);
}

#pragma endregion