#include "page_table_entry.h"

void PageTableEntry::setFrameNumber(const int &value) { m_frameNumber = value; }
int PageTableEntry::getFrameNumber() { return m_frameNumber; };
int PageTableEntry::getPageNumber() { return m_pageNumber; };
int PageTableEntry::getPid() { return m_pid; };

void PageTableEntry::setValidBit() { m_validBit = 1; };
void PageTableEntry::clearValidBit() { m_validBit = 0; }
/**
 * @brief 
 * 
 * @return true Has been mapped
 * @return false Has not been mapped
 */
bool PageTableEntry::checkValidBit() { return (bool)m_validBit; };

void PageTableEntry::setReferenceBit() { m_validBit = 1; };
void PageTableEntry::clearReferenceBit() { m_validBit = 0; }
/**
 * @brief 
 * 
 * @return true Not a candidate for eviction
 * @return false Candidate for eviction
 */
bool PageTableEntry::checkReferenceBit() { return (bool)m_validBit; };

void PageTableEntry::setDirtyBit() { m_validBit = 1; };
void PageTableEntry::clearDirtyBit() { m_validBit = 0; }
/**
 * @brief 
 * 
 * @return true Data has changed
 * @return false Data has not changed
 */
bool PageTableEntry::checkDirtyBit() { return (bool)m_validBit; };