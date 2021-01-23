#ifndef PAGE_TABLE_ENTRY_H
#define PAGE_TABLE_ENTRY_H

class PageTableEntry
{
private:
    int m_frameNumber = 0;
    int m_pageNumber;
    int m_pid;

    // 1: Has been mapped to physical address
    // 0: Has not been mapped to physical address
    char m_validBit = 0;

    // 1: Not a candidate for eviction
    // 0: Candidate for eviction
    char m_referenceBit = 0;

    // 1: Has been changed
    // 0: Has not been changed
    char m_dirtyBit = 0;

public:
    PageTableEntry(const int pageNumber, const int pid)
        : m_pageNumber(pageNumber), m_pid(pid){};

    void setFrameNumber(const int &value);
    int getFrameNumber();
    int getPageNumber();
    int getPid();

    void setValidBit();
    void clearValidBit();
    bool checkValidBit();

    void setReferenceBit();
    void clearReferenceBit();
    bool checkReferenceBit();

    void setDirtyBit();
    void clearDirtyBit();
    bool checkDirtyBit();
};

#endif