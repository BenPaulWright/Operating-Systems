#include "page_table.h"

PageTable::PageTable(const int &pid, const int &pageSize, const int &memCapacity)
{
    m_associatedPid = pid;
    m_pageSize = pageSize;
    m_virtualMemoryCapacity = memCapacity;

    if (pageSize == 0)
        throw;

    int pageCount = memCapacity / pageSize;

    m_entries.reserve(pageCount);
    for (int i = 0; i < pageCount; i++)
    {
        PageTableEntry pte(i, pid);
        m_entries.push_back(pte);
    }
}

void PageTable::AssertNotEmpty(const std::string &caller)
{
    if (m_entries.size() == 0)
        printf("ERROR: page table was empty\nCALLER: %s\n", caller.c_str());
    assert(m_entries.size() > 0);
}

PageTableEntry *PageTable::getEntry(const int &virtualAddress)
{
    AssertNotEmpty("PageTable::getEntry(const int &virtualAddress)");
    PageTableEntry *entry = &m_entries[getPageNumber(virtualAddress)];
    assert(entry != nullptr);
    return entry;
};

// Translates a virtual address to a physical address
int PageTable::getPhysicalAddress(const int &virtualAddress)
{
    AssertNotEmpty("PageTable::getPhysicalAddress(const int &virtualAddress)");
    int frame = m_entries[getPageNumber(virtualAddress)].getFrameNumber();
    int offset = virtualAddress % m_pageSize;
    return frame + offset;
};

// Returns the page number that contains the virtual address
int PageTable::getPageNumber(const int &virtualAddress)
{
    return virtualAddress / m_pageSize;
};

int PageTable::getFrameNumber(const int &virtualAddress)
{
    AssertNotEmpty("PageTable::getFrameNumber(const int &virtualAddress)");
    return m_entries[getPageNumber(virtualAddress)].getFrameNumber();
}

int PageTable::getAssociatedPid()
{
    return m_associatedPid;
};