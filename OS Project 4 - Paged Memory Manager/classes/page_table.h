#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H
#include "page_table_entry.h"
#include <vector>
#include <string>
#include <assert.h>

class PageTable
{
private:
    std::vector<PageTableEntry> m_entries;
    int m_pageSize = -1;
    int m_virtualMemoryCapacity = -1;
    int m_associatedPid = -1;

public:
    PageTable(const int &pid, const int &pageSize, const int &memCapacity);

    void AssertNotEmpty(const std::string &caller);

    // Returns the pointer to the entry corresponding to the virtual address
    PageTableEntry *getEntry(const int &virtualAddress);

    // Translates a virtual address to a physical address
    int getPhysicalAddress(const int &virtualAddress);

    // Returns the page number that contains the virtual address
    int getPageNumber(const int &virtualAddress);

    // Returns the frame number that is associated with the virtual address
    int getFrameNumber(const int &virtualAddress);

    // Returns the id of the process associated with the table
    int getAssociatedPid();
};

#endif