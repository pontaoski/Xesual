#pragma once

#include "Paging.h"

namespace VirtualMemoryManagement
{
    extern PageTableRef KernelPageTable;

    bool allocatePageForTableEntry(PageTableEntry* entry);
    void freePageForTableEntry(PageTableEntry* entry);

    PageTableRef allocateRootPageTable();
    bool allocatePageTableForTableEntry(PageTableEntry* entry);
    void map(PageTableRef root, uint64_t virtualAddress, uint64_t toPhysical);
    PageTableRef shallowCopy(PageTableRef root);
}
